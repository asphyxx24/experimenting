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
- **I2S-Mikrofon:** verlötet und funktionsfähig, **GPIO-Belegung noch nicht dokumentiert** — muss nachgetragen werden, bevor der Audio-Teil geschrieben werden kann (SCK/BCLK, WS/LRCL, SD/DOUT)

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

## Offene Punkte

- **PTT auf Hardware gegentesten** — vor allem, ob die 200-ms-Mindesthaltedauer und die 80-ms-Loslass-Karenz sich gut anfühlen
- **GPIOs des Mikrofons dokumentieren**, dann I2S-Aufnahme in den `PTT_EVENT_START`/`STOP`-Zweigen von `main.c` implementieren
- CST816S-Interrupt-Pin prüfen: aktuell wird gepollt, für die Akkulaufzeit im Idle wäre Interrupt-Betrieb besser
- Sprite-Darstellung verfeinern (Farben, Byte-Order prüfen falls Farben falsch)
- Weitere/bessere Sprite-Sheets generieren
- Sleep-Animation verbessern (zzz-Overlay)
- `idf.py monitor` braucht ein interaktives Terminal (nicht über Claude aufrufbar)
