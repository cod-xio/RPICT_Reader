/*
 * RPICT Universal Monitor
 * ========================
 * Boards  : ESP32 | ESP32-C3 | ESP8266
 * RPICT   : RPICT3V1 (7 Ch) | RPICT7V1 (15 Ch)
 * Features: Web-UI · MQTT · Static/DHCP · Hostname
 *           Konfigurierbarer RX/TX-Pin & MQTT-Intervall
 *           Energie-Akkumulator (kWh) Tag/Monat/Jahr
 *           HTTP Basic Auth
 *
 * PlatformIO-Environments:  esp32 | esp32c3 | esp8266
 */

// ── Arduino.h zuerst – verhindert Konflikte mit Serial-Makros ───────────────
#include <Arduino.h>

// ── Board-spezifische Includes ────────────────────────────────────────────────
#ifdef BOARD_ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <SoftwareSerial.h>
  #define WebServer        ESP8266WebServer
  #include <EEPROM.h>
#else
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Preferences.h>
#endif

#include <PubSubClient.h>
#include <ArduinoJson.h>

// ── Debug-Serial ─────────────────────────────────────────────────────────────
// DBG = UART0 (Hardware-Serial) auf allen Boards.
// ARDUINO_USB_CDC_ON_BOOT wird bewusst NICHT gesetzt (wuerde Serial->USBSerial
// im gesamten Framework remappen und chip-debug-report.cpp brechen).
#define DBG  Serial

#include "config.h"
#include "html.h"

// ══════════════════════════════════════════════════════════════════════════════
// Plattform-Abstraktion: Serial-Port & Preferences
// ══════════════════════════════════════════════════════════════════════════════

#ifdef BOARD_ESP8266
  // SoftwareSerial wird dynamisch mit konfigurierbaren Pins initialisiert
  SoftwareSerial* rpictSerial = nullptr;
  #define RPICT_SERIAL (*rpictSerial)

  // Einfacher NVS-Shim fuer ESP8266 via EEPROM (1 KB Fake-NVS als JSON-Block)
  #define EEPROM_SIZE 1024
  static char nvsBuf[EEPROM_SIZE];

  void nvsLoad() {
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < EEPROM_SIZE; i++) nvsBuf[i] = EEPROM.read(i);
    EEPROM.end();
    if (nvsBuf[0] != '{') { nvsBuf[0] = '{'; nvsBuf[1] = '}'; nvsBuf[2] = 0; }
  }
  void nvsSave(const char* json) {
    EEPROM.begin(EEPROM_SIZE);
    int len = strlen(json);
    if (len >= EEPROM_SIZE) len = EEPROM_SIZE - 1;
    for (int i = 0; i < len; i++) EEPROM.write(i, json[i]);
    EEPROM.write(len, 0);
    EEPROM.commit();
    EEPROM.end();
  }
  String nvsGet(const char* key, const char* def) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    return d[key] | String(def);
  }
  int nvsGetInt(const char* key, int def) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    return d[key] | def;
  }
  bool nvsGetBool(const char* key, bool def) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    return d.containsKey(key) ? (bool)d[key] : def;
  }
  float nvsGetFloat(const char* key, float def) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    return d[key] | def;
  }
  void nvsPutString(const char* key, const String& val) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    d[key] = val;
    String s; serializeJson(d, s); s.toCharArray(nvsBuf, EEPROM_SIZE);
    nvsSave(nvsBuf);
  }
  void nvsPutInt(const char* key, int val) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    d[key] = val;
    String s; serializeJson(d, s); s.toCharArray(nvsBuf, EEPROM_SIZE);
    nvsSave(nvsBuf);
  }
  void nvsPutBool(const char* key, bool val) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    d[key] = val;
    String s; serializeJson(d, s); s.toCharArray(nvsBuf, EEPROM_SIZE);
    nvsSave(nvsBuf);
  }
  void nvsPutFloat(const char* key, float val) {
    DynamicJsonDocument d(EEPROM_SIZE); deserializeJson(d, nvsBuf);
    d[key] = val;
    String s; serializeJson(d, s); s.toCharArray(nvsBuf, EEPROM_SIZE);
    nvsSave(nvsBuf);
  }
  void nvsClear() { strcpy(nvsBuf,"{}"); nvsSave(nvsBuf); }

#else
  // ESP32 / C3: Hardware-Serial + Preferences
  HardwareSerial* rpictSerial = nullptr;
  #define RPICT_SERIAL (*rpictSerial)
  Preferences prefs;

  void nvsLoad() {}  // Preferences selbst lazy
  String  nvsGet(const char* k, const char* d) { prefs.begin("rpict",true); auto v=prefs.getString(k,d); prefs.end(); return v; }
  int     nvsGetInt(const char* k, int d)    { prefs.begin("rpict",true); auto v=prefs.getInt(k,d);    prefs.end(); return v; }
  bool    nvsGetBool(const char* k, bool d)  { prefs.begin("rpict",true); auto v=prefs.getBool(k,d);   prefs.end(); return v; }
  float   nvsGetFloat(const char* k, float d){ prefs.begin("rpict",true); auto v=prefs.getFloat(k,d);  prefs.end(); return v; }
  void    nvsPutString(const char* k, const String& v){ prefs.begin("rpict",false); prefs.putString(k,v); prefs.end(); }
  void    nvsPutInt(const char* k, int v)    { prefs.begin("rpict",false); prefs.putInt(k,v);    prefs.end(); }
  void    nvsPutBool(const char* k, bool v)  { prefs.begin("rpict",false); prefs.putBool(k,v);   prefs.end(); }
  void    nvsPutFloat(const char* k, float v){ prefs.begin("rpict",false); prefs.putFloat(k,v);  prefs.end(); }
  void    nvsClear() { prefs.begin("rpict",false); prefs.clear(); prefs.end(); }
#endif

// ══════════════════════════════════════════════════════════════════════════════
// Datenstrukturen
// ══════════════════════════════════════════════════════════════════════════════

struct Config {
  // WiFi
  String wifiSSID, wifiPass;
  String hostname;
  bool   useStaticIP;
  String staticIP, gateway, subnet, dns;
  // Hardware
  int    rxPin, txPin;
  // MQTT
  String mqttHost, mqttUser, mqttPass, mqttPath;
  int    mqttPort;
  uint32_t mqttInterval;   // ms
  // RPICT
  int    model;
  String chNames[MAX_CHANNELS];
  // Web-Auth
  String webUser, webPass;
};

struct RPICTData {
  float         values[MAX_CHANNELS];  // [0..np-1]=Power [np..np+ni-1]=Irms [np+ni]=Vrms
  bool          valid;
  unsigned long lastUpdate;
  String        rawLine;
};

struct HistEntry {
  unsigned long ts;
  float         v[MAX_CHANNELS];
};

// Energie-Akkumulator (kWh pro Phase)
struct EnergyAcc {
  float  todayWh[7];    // Wh heute (max 7 Phasen)
  float  monthWh[7];
  float  yearWh[7];
  int    lastDay, lastMonth, lastYear;
  unsigned long lastMillis;
};

// ══════════════════════════════════════════════════════════════════════════════
// Globale Variablen
// ══════════════════════════════════════════════════════════════════════════════

Config      cfg;
RPICTData   rpict    = {{0}, false, 0, ""};
HistEntry   hist[HIST_SIZE];
int         histHead = 0, histCount = 0;
EnergyAcc   energy   = {{0},{0},{0}, -1,-1,-1, 0};

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);
WebServer    web(80);

unsigned long lastMqttRetry = 0;
unsigned long lastPublish   = 0;
unsigned long lastEnergySave= 0;

const char* DEF_NAMES_3V1[7]  = {"RP1","RP2","RP3","Irms1","Irms2","Irms3","Vrms"};
const char* DEF_NAMES_7V1[15] = {"RP1","RP2","RP3","RP4","RP5","RP6","RP7",
                                   "Irms1","Irms2","Irms3","Irms4","Irms5","Irms6","Irms7","Vrms"};

// ── Hilfsfunktionen ──────────────────────────────────────────────────────────
inline int numChannels(){ return cfg.model == MODEL_7V1 ? 15 : 7; }
inline int numPower()   { return cfg.model == MODEL_7V1 ?  7 : 3; }
inline int numIrms()    { return cfg.model == MODEL_7V1 ?  7 : 3; }

// ══════════════════════════════════════════════════════════════════════════════
// Config laden / speichern
// ══════════════════════════════════════════════════════════════════════════════

void loadConfig() {
  nvsLoad();
  cfg.wifiSSID    = nvsGet("wSSID",  DEF_WIFI_SSID);
  cfg.wifiPass    = nvsGet("wPASS",  DEF_WIFI_PASS);
  cfg.hostname    = nvsGet("hname",  DEF_HOSTNAME);
  cfg.useStaticIP = nvsGetBool("sIP", DEF_USE_STATIC_IP);
  cfg.staticIP    = nvsGet("sAddr",  DEF_STATIC_IP);
  cfg.gateway     = nvsGet("sGW",    DEF_GATEWAY);
  cfg.subnet      = nvsGet("sSN",    DEF_SUBNET);
  cfg.dns         = nvsGet("sDNS",   DEF_DNS);
  cfg.rxPin       = nvsGetInt("rxPin", DEF_RX_PIN);
  cfg.txPin       = nvsGetInt("txPin", DEF_TX_PIN);
  cfg.mqttHost    = nvsGet("mHOST",  DEF_MQTT_HOST);
  cfg.mqttPort    = nvsGetInt("mPORT", DEF_MQTT_PORT);
  cfg.mqttUser    = nvsGet("mUSER",  DEF_MQTT_USER);
  cfg.mqttPass    = nvsGet("mPASS",  DEF_MQTT_PASS);
  cfg.mqttPath    = nvsGet("mPATH",  DEF_MQTT_PATH);
  cfg.mqttInterval= (uint32_t)nvsGetInt("mINT", DEF_MQTT_INTERVAL);
  cfg.model       = nvsGetInt("model", DEF_MODEL);
  cfg.webUser     = nvsGet("wbUSR",  DEF_WEB_USER);
  cfg.webPass     = nvsGet("wbPSS",  DEF_WEB_PASS);

  const char** defN = (cfg.model == MODEL_7V1) ? DEF_NAMES_7V1 : DEF_NAMES_3V1;
  int n = numChannels();
  for (int i = 0; i < MAX_CHANNELS; i++) {
    char k[6]; snprintf(k, sizeof(k), "ch%d", i);
    String def = (i < n) ? String(defN[i]) : ("CH" + String(i+1));
    cfg.chNames[i] = nvsGet(k, def.c_str());
  }
}

void saveConfig() {
  nvsPutString("wSSID",  cfg.wifiSSID);
  nvsPutString("wPASS",  cfg.wifiPass);
  nvsPutString("hname",  cfg.hostname);
  nvsPutBool  ("sIP",    cfg.useStaticIP);
  nvsPutString("sAddr",  cfg.staticIP);
  nvsPutString("sGW",    cfg.gateway);
  nvsPutString("sSN",    cfg.subnet);
  nvsPutString("sDNS",   cfg.dns);
  nvsPutInt   ("rxPin",  cfg.rxPin);
  nvsPutInt   ("txPin",  cfg.txPin);
  nvsPutString("mHOST",  cfg.mqttHost);
  nvsPutInt   ("mPORT",  cfg.mqttPort);
  nvsPutString("mUSER",  cfg.mqttUser);
  nvsPutString("mPASS",  cfg.mqttPass);
  nvsPutString("mPATH",  cfg.mqttPath);
  nvsPutInt   ("mINT",   (int)cfg.mqttInterval);
  nvsPutInt   ("model",  cfg.model);
  nvsPutString("wbUSR",  cfg.webUser);
  nvsPutString("wbPSS",  cfg.webPass);
  for (int i = 0; i < MAX_CHANNELS; i++) {
    char k[6]; snprintf(k, sizeof(k), "ch%d", i);
    nvsPutString(k, cfg.chNames[i]);
  }
}

// ── Energie-Akkumulator speichern / laden ─────────────────────────────────────
void loadEnergy() {
  int np = numPower();
  for (int i = 0; i < np; i++) {
    char k[10];
    snprintf(k, sizeof(k), "eD%d", i); energy.todayWh[i] = nvsGetFloat(k, 0);
    snprintf(k, sizeof(k), "eM%d", i); energy.monthWh[i] = nvsGetFloat(k, 0);
    snprintf(k, sizeof(k), "eY%d", i); energy.yearWh[i]  = nvsGetFloat(k, 0);
  }
  energy.lastDay   = nvsGetInt("eDay",  -1);
  energy.lastMonth = nvsGetInt("eMon",  -1);
  energy.lastYear  = nvsGetInt("eYr",   -1);
}
void saveEnergy() {
  int np = numPower();
  for (int i = 0; i < np; i++) {
    char k[10];
    snprintf(k, sizeof(k), "eD%d", i); nvsPutFloat(k, energy.todayWh[i]);
    snprintf(k, sizeof(k), "eM%d", i); nvsPutFloat(k, energy.monthWh[i]);
    snprintf(k, sizeof(k), "eY%d", i); nvsPutFloat(k, energy.yearWh[i]);
  }
  nvsPutInt("eDay", energy.lastDay);
  nvsPutInt("eMon", energy.lastMonth);
  nvsPutInt("eYr",  energy.lastYear);
}

// ══════════════════════════════════════════════════════════════════════════════
// Energie-Akkumulator Update
// Wird nach jeder validen RPICT-Messung aufgerufen.
// dt_ms = Zeit seit letzter Messung in Millisekunden
// ══════════════════════════════════════════════════════════════════════════════
void updateEnergy(unsigned long dt_ms) {
  if (dt_ms == 0 || dt_ms > 60000) return;  // Plausibilitaet

  // Echtzeit via millis() – kein RTC. Datum simuliert via Laufzeit-Taege.
  // Fuer echtes Datum: NTP-Library einbinden und time() nutzen.
  unsigned long nowSec  = millis() / 1000UL;
  int  curDay   = (int)(nowSec / 86400UL);
  int  curMonth = (int)(nowSec / (86400UL * 30));
  int  curYear  = (int)(nowSec / (86400UL * 365));

  // Tageswechsel
  if (energy.lastDay != curDay) {
    if (energy.lastDay >= 0) {
      // Tageswert reset; Monat weiter akkumulieren (bereits in monthWh)
      memset(energy.todayWh, 0, sizeof(energy.todayWh));
    }
    energy.lastDay = curDay;
  }
  // Monatswechsel
  if (energy.lastMonth != curMonth) {
    if (energy.lastMonth >= 0) memset(energy.monthWh, 0, sizeof(energy.monthWh));
    energy.lastMonth = curMonth;
  }
  // Jahreswechsel
  if (energy.lastYear != curYear) {
    if (energy.lastYear >= 0) memset(energy.yearWh, 0, sizeof(energy.yearWh));
    energy.lastYear = curYear;
  }

  // Wh akkumulieren (P[W] * dt[h])
  float dt_h = dt_ms / 3600000.0f;
  int np = numPower();
  for (int i = 0; i < np && i < 7; i++) {
    float wh = rpict.values[i] * dt_h;
    if (wh < 0) wh = 0;  // keine negativen Werte
    energy.todayWh[i] += wh;
    energy.monthWh[i] += wh;
    energy.yearWh[i]  += wh;
  }
}

// ══════════════════════════════════════════════════════════════════════════════
// RPICT Serial Parser
// ══════════════════════════════════════════════════════════════════════════════
void parseRPICT(const String& line) {
  String l = line; l.trim();
  if (!l.length()) return;

  int tokenCount = 1;
  for (char c : l) if (c == ' ') tokenCount++;
  int expected = numChannels() + 1;
  if (tokenCount != expected) {
    DBG.printf("[RPICT] %d Tokens != erwartet %d\n", tokenCount, expected);
    return;
  }

  int   pos = 0, field = -1;
  float tmp[MAX_CHANNELS] = {0};
  while (pos <= (int)l.length() && field < numChannels()) {
    int sp = l.indexOf(' ', pos);
    if (sp == -1) sp = l.length();
    if (field >= 0) tmp[field] = l.substring(pos, sp).toFloat();
    field++; pos = sp + 1;
  }
  if (field < numChannels()) return;

  unsigned long now = millis();
  unsigned long dt  = (energy.lastMillis > 0 && now > energy.lastMillis)
                      ? now - energy.lastMillis : 0;

  for (int i = 0; i < numChannels(); i++) rpict.values[i] = tmp[i];
  rpict.valid      = true;
  rpict.lastUpdate = now;
  rpict.rawLine    = l;

  updateEnergy(dt);
  energy.lastMillis = now;

  hist[histHead].ts = now;
  for (int i = 0; i < numChannels(); i++) hist[histHead].v[i] = tmp[i];
  histHead = (histHead + 1) % HIST_SIZE;
  if (histCount < HIST_SIZE) histCount++;
}

// ══════════════════════════════════════════════════════════════════════════════
// MQTT
// ══════════════════════════════════════════════════════════════════════════════
bool mqttReconnect() {
  if (WiFi.status() != WL_CONNECTED) return false;
  mqtt.setServer(cfg.mqttHost.c_str(), cfg.mqttPort);
  String cid = String(BOARD_NAME) + "-RPICT-" + String(
    #ifdef BOARD_ESP8266
      ESP.getChipId()
    #else
      (uint32_t)ESP.getEfuseMac()
    #endif
    , HEX);
  return cfg.mqttUser.length()
    ? mqtt.connect(cid.c_str(), cfg.mqttUser.c_str(), cfg.mqttPass.c_str())
    : mqtt.connect(cid.c_str());
}

void mqttPublish() {
  if (!mqtt.connected()) return;
  int n = numChannels(), np = numPower();
  String base = cfg.mqttPath + "/";

  // Einzelne Messwert-Topics
  for (int i = 0; i < n; i++)
    mqtt.publish((base + cfg.chNames[i]).c_str(),
                 String(rpict.values[i], 2).c_str(), true);

  // Energie-Topics (kWh)
  for (int i = 0; i < np && i < 7; i++) {
    String eb = base + "energy/" + cfg.chNames[i] + "/";
    mqtt.publish((eb + "today_kWh").c_str(),  String(energy.todayWh[i]/1000.0f, 4).c_str(), true);
    mqtt.publish((eb + "month_kWh").c_str(),  String(energy.monthWh[i]/1000.0f, 4).c_str(), true);
    mqtt.publish((eb + "year_kWh").c_str(),   String(energy.yearWh[i]/1000.0f,  4).c_str(), true);
  }

  // JSON-Sammel-Topic
  DynamicJsonDocument doc(1024);
  for (int i = 0; i < n; i++) doc[cfg.chNames[i]] = rpict.values[i];
  JsonObject en = doc.createNestedObject("energy_kWh");
  for (int i = 0; i < np && i < 7; i++) {
    JsonObject ph = en.createNestedObject(cfg.chNames[i]);
    ph["today"]  = energy.todayWh[i]/1000.0f;
    ph["month"]  = energy.monthWh[i]/1000.0f;
    ph["year"]   = energy.yearWh[i]/1000.0f;
  }
  char buf[1024]; serializeJson(doc, buf, sizeof(buf));
  mqtt.publish((cfg.mqttPath + "/json").c_str(), buf, true);
}

// ══════════════════════════════════════════════════════════════════════════════
// WiFi Setup (Static IP / DHCP / Hostname)
// ══════════════════════════════════════════════════════════════════════════════
void wifiSetup() {
  WiFi.mode(WIFI_STA);

  // Hostname setzen
  #ifdef BOARD_ESP8266
    WiFi.hostname(cfg.hostname);
  #else
    WiFi.setHostname(cfg.hostname.c_str());
  #endif

  // Static IP konfigurieren
  if (cfg.useStaticIP) {
    IPAddress ip, gw, sn, dns;
    if (ip.fromString(cfg.staticIP) && gw.fromString(cfg.gateway) &&
        sn.fromString(cfg.subnet)   && dns.fromString(cfg.dns)) {
      WiFi.config(ip, gw, sn, dns);
      DBG.printf("[WiFi] Static IP: %s GW: %s\n", cfg.staticIP.c_str(), cfg.gateway.c_str());
    } else {
      DBG.println("[WiFi] Static IP Parsing-Fehler -> DHCP");
    }
  }

  WiFi.begin(cfg.wifiSSID.c_str(), cfg.wifiPass.c_str());
  DBG.printf("[WiFi] Verbinde mit '%s'", cfg.wifiSSID.c_str());
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < WIFI_TIMEOUT_MS) {
    delay(250); DBG.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    DBG.printf(" OK\n[WiFi] IP: %s  Hostname: %s\n",
      WiFi.localIP().toString().c_str(), cfg.hostname.c_str());
  } else {
    DBG.println("\n[WiFi] Timeout → AP-Modus");
    WiFi.softAP(AP_SSID, AP_PASS);
    DBG.printf("[AP] SSID: %s  Pass: %s  IP: %s\n",
      AP_SSID, AP_PASS, WiFi.softAPIP().toString().c_str());
  }
}

// ══════════════════════════════════════════════════════════════════════════════
// Web-Auth
// ══════════════════════════════════════════════════════════════════════════════
bool requireAuth() {
  if (!web.authenticate(cfg.webUser.c_str(), cfg.webPass.c_str())) {
    web.requestAuthentication(BASIC_AUTH, "RPICT Monitor", "Login erforderlich");
    return false;
  }
  return true;
}

// ══════════════════════════════════════════════════════════════════════════════
// Web-Handler
// ══════════════════════════════════════════════════════════════════════════════
void handleRoot() {
  if (!requireAuth()) return;
  web.send_P(200, "text/html", HTML);
}

void handleApiData() {
  if (!requireAuth()) return;
  int n = numChannels(), np = numPower();
  DynamicJsonDocument doc(3072);

  doc["wifi"]     = (WiFi.status() == WL_CONNECTED);
  doc["mqtt"]     = mqtt.connected();
  doc["ip"]       = WiFi.localIP().toString();
  doc["hostname"] = cfg.hostname;
  doc["valid"]    = rpict.valid;
  doc["raw"]      = rpict.rawLine;
  doc["model"]    = cfg.model;
  doc["board"]    = BOARD_NAME;

  // Messwerte
  JsonArray vals = doc.createNestedArray("values");
  for (int i = 0; i < n; i++) vals.add(rpict.values[i]);

  // Kanal-Namen
  JsonArray nm = doc.createNestedArray("names");
  for (int i = 0; i < n; i++) nm.add(cfg.chNames[i]);

  // Energie kWh (pro Phase: today/month/year)
  JsonArray enArr = doc.createNestedArray("energy");
  for (int i = 0; i < np && i < 7; i++) {
    JsonObject ph = enArr.createNestedObject();
    ph["today"] = energy.todayWh[i] / 1000.0f;
    ph["month"] = energy.monthWh[i] / 1000.0f;
    ph["year"]  = energy.yearWh[i]  / 1000.0f;
  }

  // Konfig (kein Passwort!)
  JsonObject cfgJ     = doc.createNestedObject("cfg");
  cfgJ["wSSID"]       = cfg.wifiSSID;
  cfgJ["hname"]       = cfg.hostname;
  cfgJ["sIP"]         = cfg.useStaticIP;
  cfgJ["sAddr"]       = cfg.staticIP;
  cfgJ["sGW"]         = cfg.gateway;
  cfgJ["sSN"]         = cfg.subnet;
  cfgJ["sDNS"]        = cfg.dns;
  cfgJ["rxPin"]       = cfg.rxPin;
  cfgJ["txPin"]       = cfg.txPin;
  cfgJ["mHOST"]       = cfg.mqttHost;
  cfgJ["mPORT"]       = cfg.mqttPort;
  cfgJ["mUSER"]       = cfg.mqttUser;
  cfgJ["mPATH"]       = cfg.mqttPath;
  cfgJ["mINT"]        = cfg.mqttInterval;
  cfgJ["model"]       = cfg.model;
  cfgJ["webUser"]     = cfg.webUser;
  JsonArray chJ       = cfgJ.createNestedArray("chNames");
  for (int i = 0; i < n; i++) chJ.add(cfg.chNames[i]);

  // History
  JsonArray histJ = doc.createNestedArray("history");
  int start = (histCount < HIST_SIZE) ? 0 : histHead;
  for (int j = 0; j < histCount; j++) {
    int idx = (start + j) % HIST_SIZE;
    JsonObject e = histJ.createNestedObject();
    e["ts"] = hist[idx].ts;
    JsonArray hv = e.createNestedArray("v");
    for (int k = 0; k < n; k++) hv.add(hist[idx].v[k]);
  }

  String out; serializeJson(doc, out);
  web.send(200, "application/json", out);
}

void handleApiConfig() {
  if (!requireAuth()) return;
  if (web.method() != HTTP_POST) { web.send(405); return; }

  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, web.arg("plain"))) { web.send(400,"text/plain","Bad JSON"); return; }

  cfg.wifiSSID    = doc["wSSID"]  | cfg.wifiSSID;
  String nWP      = doc["wPASS"]  | String("");   if (nWP.length())  cfg.wifiPass    = nWP;
  cfg.hostname    = doc["hname"]  | cfg.hostname;
  cfg.useStaticIP = doc["sIP"]    | cfg.useStaticIP;
  cfg.staticIP    = doc["sAddr"]  | cfg.staticIP;
  cfg.gateway     = doc["sGW"]    | cfg.gateway;
  cfg.subnet      = doc["sSN"]    | cfg.subnet;
  cfg.dns         = doc["sDNS"]   | cfg.dns;
  cfg.rxPin       = doc["rxPin"]  | cfg.rxPin;
  cfg.txPin       = doc["txPin"]  | cfg.txPin;
  cfg.mqttHost    = doc["mHOST"]  | cfg.mqttHost;
  cfg.mqttPort    = doc["mPORT"]  | cfg.mqttPort;
  cfg.mqttUser    = doc["mUSER"]  | cfg.mqttUser;
  String nMP      = doc["mPASS"]  | String("");   if (nMP.length())  cfg.mqttPass    = nMP;
  cfg.mqttPath    = doc["mPATH"]  | cfg.mqttPath;
  cfg.mqttInterval= doc["mINT"]   | cfg.mqttInterval;
  cfg.model       = doc["model"]  | cfg.model;
  String nWU      = doc["wbUSR"]  | String("");   if (nWU.length())  cfg.webUser     = nWU;
  String nWW      = doc["wbPSS"]  | String("");   if (nWW.length())  cfg.webPass     = nWW;

  JsonArray chArr = doc["chNames"].as<JsonArray>();
  if (chArr) { int i=0; for (JsonVariant v: chArr) if(i<MAX_CHANNELS) cfg.chNames[i++]=v.as<String>(); }

  saveConfig();
  web.send(200, "text/plain", "OK");
  delay(300);
  ESP.restart();
}

void handleApiEnergyReset() {
  if (!requireAuth()) return;
  memset(&energy, 0, sizeof(energy));
  energy.lastDay = energy.lastMonth = energy.lastYear = -1;
  saveEnergy();
  web.send(200, "text/plain", "Energy reset OK");
}

void handleReset() {
  if (!requireAuth()) return;
  nvsClear();
  web.send(200, "text/plain", "Factory reset – restarting...");
  delay(300);
  ESP.restart();
}

// ══════════════════════════════════════════════════════════════════════════════
// Serial-Port initialisieren (nach Config-Load, da Pins konfigurierbar)
// ══════════════════════════════════════════════════════════════════════════════
void initRPICTSerial() {
#ifdef BOARD_ESP8266
  if (rpictSerial) { rpictSerial->end(); delete rpictSerial; }
  rpictSerial = new SoftwareSerial(cfg.rxPin, cfg.txPin);
  rpictSerial->begin(RPICT_BAUD);
  DBG.printf("[HW] SoftwareSerial RX=GPIO%d TX=GPIO%d\n", cfg.rxPin, cfg.txPin);
#else
  // ESP32 / C3: UART1 fuer C3, UART2 fuer ESP32
  #ifdef BOARD_ESP32_C3
    if (!rpictSerial) rpictSerial = new HardwareSerial(1);
  #else
    if (!rpictSerial) rpictSerial = new HardwareSerial(2);
  #endif
  rpictSerial->begin(RPICT_BAUD, SERIAL_8N1, cfg.rxPin, cfg.txPin);
  DBG.printf("[HW] HardwareSerial RX=GPIO%d TX=GPIO%d\n", cfg.rxPin, cfg.txPin);
#endif
}

// ══════════════════════════════════════════════════════════════════════════════
// Setup
// ══════════════════════════════════════════════════════════════════════════════
void setup() {
  DBG.begin(115200);
  delay(200);
  DBG.println("\n╔════════════════════════════════╗");
  DBG.println("║  RPICT Universal Monitor       ║");
  DBG.printf ("║  Board: %-22s ║\n", BOARD_NAME);
  DBG.println("╚════════════════════════════════╝");

  loadConfig();
  loadEnergy();

  DBG.printf("[CFG] Modell: %s (%d Kanaele)\n",
    cfg.model==MODEL_7V1?"RPICT7V1":"RPICT3V1", numChannels());
  DBG.printf("[CFG] MQTT : %s:%d  Intervall: %ums\n",
    cfg.mqttHost.c_str(), cfg.mqttPort, cfg.mqttInterval);
  DBG.printf("[CFG] Web  : Benutzer »%s«\n", cfg.webUser.c_str());

  initRPICTSerial();
  wifiSetup();

  web.on("/",                   handleRoot);
  web.on("/api/data",           handleApiData);
  web.on("/api/config",  HTTP_POST, handleApiConfig);
  web.on("/api/energy/reset", HTTP_POST, handleApiEnergyReset);
  web.on("/reset",              handleReset);
  web.begin();
  DBG.println("[WEB] Server auf Port 80 gestartet");

  mqtt.setServer(cfg.mqttHost.c_str(), cfg.mqttPort);
  mqtt.setBufferSize(1024);
}

// ══════════════════════════════════════════════════════════════════════════════
// Loop
// ══════════════════════════════════════════════════════════════════════════════
String serialBuf = "";

void loop() {
  web.handleClient();
  mqtt.loop();

  // ── RPICT Serial lesen ───────────────────────────────────────────────────
  while (rpictSerial && RPICT_SERIAL.available()) {
    char c = (char)RPICT_SERIAL.read();
    if (c == '\n')       { parseRPICT(serialBuf); serialBuf = ""; }
    else if (c != '\r')  { if (serialBuf.length() < 256) serialBuf += c; }
  }

  // ── MQTT Reconnect ────────────────────────────────────────────────────────
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
    if (millis() - lastMqttRetry > 5000) {
      lastMqttRetry = millis();
      DBG.print("[MQTT] Verbinde...");
      DBG.println(mqttReconnect() ? " OK" : " Fehler");
    }
  }

  // ── MQTT Publish (konfigurierbares Intervall) ──────────────────────────────
  if (rpict.valid && mqtt.connected()) {
    if (millis() - lastPublish > cfg.mqttInterval) {
      lastPublish = millis();
      mqttPublish();
    }
  }

  // ── Energie-Zaehler periodisch speichern ──────────────────────────────────
  if (millis() - lastEnergySave > ENERGY_SAVE_INTERVAL) {
    lastEnergySave = millis();
    saveEnergy();
  }
}
