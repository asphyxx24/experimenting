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
Set-Location C:\Users\anton\Desktop\projects\experimenting\ideas\companion-watch\firmware
idf.py build
idf.py -p COM5 flash
```

## Pin-Belegung (verifiziert)

- **LCD SPI:** MOSI=11, SCLK=10, CS=9, DC=8, RST=14, Backlight=2
- **Touch (CST816S):** I2C SDA=6, SCL=7, RST=13, Addr=0x15
- **IMU (QMI8658):** gleicher I2C-Bus (GPIO 6/7)

## Projekt-Struktur

```
firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
└── main/
    ├── main.c              ← Hauptloop, Touch→Animation-Switch
    ├── display.c/h         ← GC9A01 via SPI
    ├── touch.c/h           ← CST816S Tap-Erkennung
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

## Aktueller Stand (2026-05-11)

- Display (GC9A01) funktioniert ✓
- Touch (CST816S) Tap-Erkennung funktioniert ✓
- Sprite-Animation mit Tap-Umschaltung funktioniert ✓
- Pig-Sprites aus Retro-Diffusion-Sheet geladen — funktioniert grundsätzlich, aber noch nicht 100% (Farben/Darstellung noch nicht perfekt)
- Placeholder-Sprites sind noch im Code als Fallback (sprite_engine.c)

## Offene Punkte

- Sprite-Darstellung verfeinern (Farben, Byte-Order prüfen falls Farben falsch)
- Weitere/bessere Sprite-Sheets generieren
- Sleep-Animation verbessern (zzz-Overlay)
- `idf.py monitor` braucht ein interaktives Terminal (nicht über Claude aufrufbar)
