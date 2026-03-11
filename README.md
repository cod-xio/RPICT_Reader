# RPICT Universal Monitor

Web-UI + MQTT-Publisher für **RPICT3V1** und **RPICT7V1**.  
Unterstützte Boards: **ESP32** | **ESP32-C3** | **ESP8266**

---

## Board-Auswahl & Verdrahtung

| PlatformIO-Env | Board | RPICT TX → anschließen an | Serial-Typ |
|---|---|---|---|
| `esp32`   | ESP32 (WROOM, DevKit v1) | GPIO **16** (Standard, änderbar) | HardwareSerial (UART2) |
| `esp32c3` | ESP32-C3 (DevKitM, SuperMini) | GPIO **4** (Standard, änderbar) | HardwareSerial (UART1) |
| `esp8266` | NodeMCU v2/v3, Wemos D1 Mini | GPIO **14** = D5 (Standard, änderbar) | SoftwareSerial |

> **Alle GPIO-Pins sind in der Web-UI unter Settings → Hardware konfigurierbar.**  
> GND des RPICT immer mit GND des ESP verbinden.

**ESP8266 Pin-Hinweise:**
- GPIO 0, 2, 15 sind Boot-Pins → nicht für RPICT verwenden  
- GPIO 19/20 auf ESP32-C3 sind USB-JTAG → nicht verwenden

---

## Features

- 📊 **Live-Dashboard** – Momentanwerte pro Phase: Volt, Ampere, Watt
- ⚡ **Energie-Akkumulator** – kWh-Zähler Tag / Monat / Jahr pro Phase
- 📈 **Charts** – Verlaufsdiagramme (letzten 60 Messungen)
- 🌐 **Web-Konfiguration** – alle Parameter über Browser einstellbar:
  - WiFi SSID/Passwort
  - Hostname (mDNS)
  - **Static IP oder DHCP** (IP, Gateway, Subnetz, DNS)
  - **RX/TX-GPIO-Pins** für RPICT
  - MQTT Broker, Port, User, Passwort, Topic-Pfad
  - **MQTT Publish-Intervall** (500 ms – 300 s)
  - RPICT-Modell (3V1 / 7V1)
  - Kanal-Namen frei benennbar
  - Web-UI Passwort (HTTP Basic Auth)
- 🔒 **Passwortschutz** – HTTP Basic Auth auf allen Routen
- 📡 **MQTT** – Einzeltopics + JSON-Sammel-Topic + Energie-Topics
- 💾 **Persistenz** – alle Einstellungen & Energiezähler im Flash (NVS/EEPROM)

---

## Erste Inbetriebnahme

1. PlatformIO-Environment wählen und flashen
2. Falls kein WiFi konfiguriert → AP-Modus:
   - **SSID:** `RPICT-Setup` | **Passwort:** `rpict1234`
   - Browser: `http://192.168.4.1`
3. Standard-Login: **admin** / **rpict1234** → **sofort ändern!**
4. Settings → WiFi, IP, MQTT, Pins, Modell einstellen → Speichern

---

## MQTT Topics

```
RPICT/RP1          → Wirkleistung Phase 1 [W]
RPICT/Vrms         → Netzspannung [V]
RPICT/energy/RP1/today_kWh  → Energie heute [kWh]
RPICT/energy/RP1/month_kWh  → Energie Monat [kWh]
RPICT/energy/RP1/year_kWh   → Energie Jahr  [kWh]
RPICT/json         → Alle Werte als JSON (inkl. energy_kWh)
```

---

## GitHub hochladen

```bash
pip install PyGithub
python upload_to_github.py
```

---

## Projektstruktur

```
RPICT_ESP32/
├── platformio.ini          ← esp32 / esp32c3 / esp8266
├── include/
│   ├── config.h            ← Board-Erkennung, Defaults, Pin-Defaults
│   └── html.h              ← Web-UI (PROGMEM)
├── src/
│   └── main.cpp            ← Firmware
├── upload_to_github.py     ← GitHub-Upload-Script
└── README.md
```
