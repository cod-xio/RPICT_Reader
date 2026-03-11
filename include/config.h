#pragma once

// ══════════════════════════════════════════════════════════════════════════════
// Board-Erkennung (wird von PlatformIO via build_flags gesetzt)
// ══════════════════════════════════════════════════════════════════════════════
#if !defined(BOARD_ESP32) && !defined(BOARD_ESP32_C3) && !defined(BOARD_ESP8266)
  #define BOARD_ESP32   // Fallback
#endif

#if defined(BOARD_ESP8266)
  #define BOARD_NAME "ESP8266"
#elif defined(BOARD_ESP32_C3)
  #define BOARD_NAME "ESP32-C3"
#else
  #define BOARD_NAME "ESP32"
#endif

// ══════════════════════════════════════════════════════════════════════════════
// Serial-Port Defaults je Board
// (RX/TX-Pins sind zur Laufzeit in der App konfigurierbar – diese Werte
//  gelten nur beim allerersten Start bzw. nach Factory-Reset)
// ══════════════════════════════════════════════════════════════════════════════
#if defined(BOARD_ESP8266)
  // ESP8266: SoftwareSerial auf D5(GPIO14)/D6(GPIO12)
  #define DEF_RX_PIN   14
  #define DEF_TX_PIN   12
#elif defined(BOARD_ESP32_C3)
  // ESP32-C3: UART1 – GPIO19/20 sind USB-JTAG, also GPIO4/5 nehmen
  #define DEF_RX_PIN    4
  #define DEF_TX_PIN    5
#else
  // Original ESP32: UART2
  #define DEF_RX_PIN   16
  #define DEF_TX_PIN   17
#endif

#define RPICT_BAUD  38400

// ══════════════════════════════════════════════════════════════════════════════
// RPICT Modell-Konstanten
// ══════════════════════════════════════════════════════════════════════════════
#define MODEL_3V1     0   //  3x Power + 3x Irms + Vrms =  7 Kanaele
#define MODEL_7V1     1   //  7x Power + 7x Irms + Vrms = 15 Kanaele
#define MAX_CHANNELS  15

// ══════════════════════════════════════════════════════════════════════════════
// Default-Werte (alle ueberschreibbar via Web-UI → werden in NVS gespeichert)
// ══════════════════════════════════════════════════════════════════════════════
#define DEF_WIFI_SSID      "MyWiFi"
#define DEF_WIFI_PASS      "MyPassword"
#define DEF_HOSTNAME       "rpict-monitor"
#define DEF_USE_STATIC_IP  false          // false = DHCP
#define DEF_STATIC_IP      "192.168.1.200"
#define DEF_GATEWAY        "192.168.1.1"
#define DEF_SUBNET         "255.255.255.0"
#define DEF_DNS            "8.8.8.8"

#define DEF_MQTT_HOST      "192.168.1.100"
#define DEF_MQTT_PORT      1883
#define DEF_MQTT_USER      ""
#define DEF_MQTT_PASS      ""
#define DEF_MQTT_PATH      "RPICT"
#define DEF_MQTT_INTERVAL  5000           // ms zwischen MQTT-Publishes

#define DEF_MODEL          MODEL_3V1

// ── Web-Auth ─────────────────────────────────────────────────────────────────
#define DEF_WEB_USER  "admin"
#define DEF_WEB_PASS  "rpict1234"

// ── AP Fallback ───────────────────────────────────────────────────────────────
#define AP_SSID  "RPICT-Setup"
#define AP_PASS  "rpict1234"

// ── Timing ───────────────────────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS   12000
#define HIST_SIZE         60

// ── Energie-Akkumulator: Persistenz-Intervall (ms) ───────────────────────────
// Alle 5 Minuten werden kWh-Zaehler in NVS gespeichert
#define ENERGY_SAVE_INTERVAL  300000UL
