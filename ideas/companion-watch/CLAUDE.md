# Companion-Watch — Dev-Hinweise

## ESP-IDF Umgebung

- **Version:** ESP-IDF v5.5.4 (Prototyp), finale Version später v6.0
- **Installation:** `C:\esp\esp-idf`
- **Umgebung laden (jede neue Shell):** `& C:\esp\esp-idf\export.ps1`
- **Target:** ESP32-S3
- **Board:** Waveshare ESP32-S3-Touch-LCD-1.28, USB-Serial auf COM5

## Build & Flash

```powershell
& C:\esp\esp-idf\export.ps1
Set-Location C:\Users\anton\Desktop\experimenting\ideas\companion-watch\firmware
idf.py build
idf.py -p COM5 flash
```

> **Achtung Build-Last:** Ein Full-Build übersetzt ~1080 Targets. In Kombination mit Windows Defenders Echtzeitschutz (jede `.o`-Datei wird gescannt) treibt das die CPU auf 100 % — gemessen 56 % allein für die Antimalware Service Executable. Vor einem Full-Build ankündigen; inkrementelle Builds sind unkritisch. Defender-Ausschlüsse für `C:\esp`, `C:\Users\anton\.espressif` und dieses Repo entschärfen das (brauchen Admin-Rechte).

## Pin-Belegung (verifiziert)

- **LCD SPI:** MOSI=11, SCLK=10, CS=9, DC=8, RST=14, Backlight=2
- **Touch (CST816S):** I2C SDA=6, SCL=7, RST=13, Addr=0x15
- **IMU (QMI8658):** gleicher I2C-Bus (GPIO 6/7)
- **I2S-Mikrofon (INMP441):** WS=GPIO15, SCK=GPIO16, SD=GPIO17 — verifiziert funktionierend (2026-05-16)

### Vollständige Pin-Belegung (aus Waveshare Wiki verifiziert)

#### Belegt — on-board Peripherie

| GPIO | Funktion | Chip/Zweck |
|------|----------|------------|
| GPIO1 | Battery ADC | Spannungsteiler 200K/100K → `3.3 / 4096 * 3 * ADC_Value` ergibt Akkuspannung |
| GPIO2 | LCD_BL | Backlight-PWM |
| GPIO3 | IMU INT2 | QMI8658 Interrupt 2 |
| GPIO4 | IMU INT1 + MOSFET1_CS | QMI8658 Interrupt 1 + Board-MOSFET für kleine Lasten |
| GPIO5 | TP_INT + MOSFET2_CS | CST816S Touch-Interrupt + zweiter Board-MOSFET |
| GPIO6 | TP_SDA / IMU SDA | I2C Bus (CST816S + QMI8658) |
| GPIO7 | TP_SCL / IMU SCL | I2C Bus (CST816S + QMI8658) |
| GPIO8 | LCD_DC | GC9A01 SPI |
| GPIO9 | LCD_CS | GC9A01 SPI |
| GPIO10 | LCD_CLK | GC9A01 SPI |
| GPIO11 | LCD_MOSI | GC9A01 SPI |
| GPIO12 | LCD_MISO | GC9A01 SPI |
| GPIO13 | TP_RST | CST816S Reset |
| GPIO14 | LCD_RST | GC9A01 Reset |
| GPIO43 | UART_TXD | CH343P USB-UART (Flash/Log) |
| GPIO44 | UART_RXD | CH343P USB-UART (Flash/Log) |

#### Frei — GPIO_OUT Connector P2 (12-Pin)

**Achtung: der Connector ist 12-Pin (NICHT 6-Pin wie zuvor dokumentiert).** Pin 1 ist mit "G1" auf dem Stecker-Gehäuse markiert (links). Verifiziert aus dem Schaltplan (files.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.28/ESP32-S3-Touch-LCD-1.28-Sch.pdf).

| Pin | Signal | Hinweis |
|-----|--------|---------|
| 1 | GND | |
| 2 | VSYS | 5V — NICHT für 3.3V-Peripherie nutzen! |
| 3 | RUN (Reset) | |
| 4 | BOOT (GPIO0) | |
| 5 | GND | |
| 6 | ADC_AVDD / 3V3 | sichere 3.3V für Sensoren |
| 7 | GPIO15 | |
| 8 | GPIO16 | |
| 9 | GPIO17 | |
| 10 | GPIO18 | |
| 11 | GPIO21 | |
| 12 | GPIO33 | |

#### INMP441 I2S Mikrofon — verifiziert funktionierend (2026-05-16)

Verkabelt über das mitgelieferte 12-Pin-Kabel an P2:

| INMP441-Pad | Kabel-Pin (von links) | Signal |
|-------------|----------------------|--------|
| L/R | 1 (G1) | GND → linker Kanal (Mono) |
| GND | 5 | GND |
| VDD | 6 | 3V3 (ADC_AVDD) |
| WS | 7 | GPIO15 |
| SCK | 8 | GPIO16 |
| SD | 9 | GPIO17 |

**I2S-Konfiguration (kritisch):**

- Standard mode, Philips slot, 32-bit width, MONO slot mode
- **`slot_mask = I2S_STD_SLOT_LEFT` zwingend setzen** — sonst liest der ESP32 den falschen Kanal und das Audio fiept nur
- Sample rate 16 kHz, oberen 16 Bit aus dem 32-Bit-Wort nehmen: `(int16_t)(raw[i] >> 16)`
- **DC-Offset entfernen** vor dem Speichern: INMP441 hat erheblichen DC-Anteil, sonst saturiert das Signal komplett (Werte nahe Int16-Max)

**WAV-Capture-Tool:** `aufnahme.py` im Projektroot — startet Recording über Serial, speichert mit Zeitstempel im Dateinamen. Voraussetzung: idf monitor muss geschlossen sein, Buffer wird beim Start geleert.

#### Vibrationsmotor — Board-MOSFETs nutzen

Das Board hat zwei Schaltstufen (MOSFETs) bereits eingebaut, Lötpads rund um den Akkuhalter:
- **MOSFET1:** GPIO4 steuert → direkt Vibrationsmotor anlöten, kein Transistor nötig
- **MOSFET2:** GPIO5 steuert → zweite Schaltstufe (z.B. IR-LED oder RGB-LED)

Achtung: GPIO4/5 sind gleichzeitig IMU INT1/INT2 und TP_INT — Konflikt prüfen wenn IMU-Interrupts aktiv genutzt werden.

## Projekt-Struktur

```
firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
└── main/
    ├── main.c              ← Hauptloop, PTT-Events + Animation-Switch
    ├── display.c/h         ← GC9A01 via SPI
    ├── touch.c/h           ← CST816S: Tap-Flanke + Halte-Zustand
    ├── ptt.c/h             ← Push-to-Talk-Zustandsautomat (Touchscreen)
    ├── sprite_engine.c/h   ← Animation-Engine, Nearest-Neighbor ×4
    ├── pig_sprites.h        ← generierte Sprite-Daten (RGB565)
    └── idf_component.yml
tools/
├── png_to_sprite.py        ← PNG → C-Header Converter (generisch)
└── generate_pig.py         ← Pig-Sprites aus Sprite-Sheet generieren
```

## Sprites neu generieren

```powershell
python tools\generate_pig.py
```
Liest `C:\Users\anton\Downloads\Minecraft_pig_walkin...-888999626-0.png`, erzeugt `firmware/main/pig_sprites.h` mit 3 Animationen (idle, walk, sleep).

## Aktueller Stand (2026-08-07)

- Display (GC9A01) funktioniert ✓
- Touch (CST816S) Tap-Erkennung funktioniert ✓
- Sprite-Animation mit Tap-Umschaltung funktioniert ✓
- I2S-Mikrofon verlötet und funktionsfähig ✓ (Pins noch nicht dokumentiert)
- **Push-to-Talk über Touchscreen implementiert — kompiliert, aber noch nicht auf Hardware getestet**
- Pig-Sprites aus Retro-Diffusion-Sheet geladen — funktioniert grundsätzlich, aber noch nicht 100% (Farben/Darstellung noch nicht perfekt)
- Placeholder-Sprites sind noch im Code als Fallback (sprite_engine.c)
- **INMP441-Mikrofon angeschlossen und verifiziert funktionierend ✓** — WAV-Aufnahme über `aufnahme.py` testweise gelaufen, Sprache klar erkennbar

## Offene Punkte

- **PTT auf Hardware gegentesten** — vor allem, ob die 200-ms-Mindesthaltedauer und die 80-ms-Loslass-Karenz sich gut anfühlen
- **GPIOs des Mikrofons dokumentieren**, dann I2S-Aufnahme in den `PTT_EVENT_START`/`STOP`-Zweigen von `main.c` implementieren
- CST816S-Interrupt-Pin prüfen: aktuell wird gepollt, für die Akkulaufzeit im Idle wäre Interrupt-Betrieb besser
- Sprite-Darstellung verfeinern (Farben, Byte-Order prüfen falls Farben falsch)
- Weitere/bessere Sprite-Sheets generieren
- Sleep-Animation verbessern (zzz-Overlay)
- Audio über WiFi statt Serial zum PC streamen (Spike D — Jarvis-Voice)
- MAX98357A + Speaker für Wiedergabe verlöten
- `idf.py monitor` braucht ein interaktives Terminal (nicht über Claude aufrufbar)
