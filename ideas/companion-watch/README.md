# Companion-Watch — Selbstbau-Smartwatch mit Jarvis-Anbindung

**Status:** 🔬 Recherche
**Letztes Update:** 2026-05-06

## Idee

Eine selbstgebaute Smartwatch auf ESP32-S3-Basis, die als universelles Companion-Device am Handgelenk dient. Kern-Features:

- **Jarvis-Voice (Push-to-Talk):** Knopf drücken → Audio-Stream zum Jarvis-Backend → Antwort hören.
- **IR-Universal-Remote:** TV, HiFi, Klima vom Handgelenk steuern.
- **Health-Tracking:** Schritte, Puls, SpO2, Schlaf — ersetzt das Xiaomi Mi Band.
- **Notifications:** Backend pusht Meldungen → Display + Vibration.

Langfristig Plattform für verschiedene UIs und Modi (z. B. [Habit-Tamagotchi](../habit-tamagotchi/README.md) als erste Zielplattform für ein Pet-UI).

## Hardware-Entscheidung

**Gewählt: Waveshare ESP32-S3-Touch-LCD-1.28** (~25 €).

Upgrade gegenüber dem non-Touch-Modell (~20 €): kapazitiver Touchscreen on-board, kaum Aufpreis, alles andere identisch.

Begründung gegen die Alternativen:
- Gegen E-Ink-Varianten: zu langsam für flüssige UI-Animation.
- Gegen Sharp Memory LCD: kleinere Auswahl an fertigen Cases.
- Gegen RP2040-Variante: kein WiFi/BLE — Killer für Voice + Sync.
- Gegen LilyGo T-Watch-S3: teurer (~60 €), weniger flexibel beim Bauteil-Layout.
- Gegen ESP32-S3-LCD-1.85: rund, mehr Pixel (360x360), aber zu groß für eine Uhr.
- Gegen ESP32-S3-Touch-AMOLED-2.06: Mikro + Speaker on-board (gut), aber fast keine freien GPIOs — IR, Vibration, RGB-LED kaum unterzubringen. Außerdem rechteckig.
- Gegen ESP32-C3/C6-Varianten: single-core, zu schwach für UI + Voice-Streaming gleichzeitig.

ESP32-S3: WiFi + BLE 5.0, 240 MHz Dual-Core, IMU on-board (QMI8658), USB-C-Charging, 65k-Farben-IPS-Display 240x240.

**Komplette Bauteile-Liste:** siehe [`BAUTEILE.md`](BAUTEILE.md).

## Mi-Band-Ersatz — Trade-offs

| | Mi Band 8 | Companion-Watch | Bewertung |
|--|-----------|-----------------|-----------|
| Akku | 14 Tage | 2–3 Tage | Mi Band gewinnt klar |
| Schritte | ja | ja (IMU) | gleich |
| Puls 24/7 | ja | ja (MAX30102) | gleich |
| Schlaf-Stages | ja | ja (Algo-Port nötig) | gleich |
| SpO2 | ja | ja | gleich |
| Voice/Jarvis-PTT | nein | ja | unser Vorteil |
| IR-Remote | nein | ja | unser Vorteil |
| Erweiterbare UIs | nein | ja | unser Vorteil |
| Tragekomfort | super dünn | klotzig (~18 mm) | Mi Band gewinnt |
| Daten offen / hackbar | proprietär | komplett offen | unser Vorteil |

**Konsequenz:** ersetzt die Mi Band, wenn man die Lade-Routine alle 2–3 Tage und die Bauhöhe akzeptiert. Sleep-Staging-Algorithmen müssen portiert werden (z. B. Cole-Kripke, [pyActigraphy](https://github.com/ghammad/pyActigraphy), [Bangle.js Sleep-Logger](https://github.com/espruino/BangleApps/tree/master/apps/sleeplog)).

## Architektur

```
[Jarvis-Backend] ──WiFi/WebSocket──> [ESP32-S3 Companion am Handgelenk]
        |                                       |
        |                                       | Display: UI (Pet, Remote, Status)
        | Datenquellen + Push                   | Audio I2S in/out
        |                                       | IR-Blaster
        v                                       | IMU + PPG
[Health Connect / Mi Fit / Strava /             |
 GitHub / Calendar / Custom-API]                v
                                       [User: Tap, PTT, Schlaf, ...]
```

Das Gerät ist **dummer Client mit IO**, nicht eigenständige AI. Modi:
1. **Idle:** UI-Animation, IMU lauscht, WiFi off.
2. **Voice:** PTT gedrückt → WiFi connect → Audio-Stream zu Jarvis → Antwort.
3. **Remote:** Display zeigt Buttons, IR/BLE/WiFi-Calls feuern.
4. **Notification:** Backend pusht → Display + Vibration.
5. **Sleep:** Display off, IMU + PPG samplen, alles in Flash, morgens syncen.

## Offene Fragen

- [ ] Watch-Case: bestehendes STL ([Watch-Case mit Akku-Slot](https://www.printables.com/model/484236)) reicht erstmal, oder direkt eigener Custom-Print?
- [ ] PTT-Hardware: Side-Push am Gehäuse vs. Touchscreen-Hold vs. IMU-Geste? → Knopf am verlässlichsten.
- [ ] Backend-Push-Protokoll: WebSocket dauerhaft offen vs. MQTT mit Wakeup-Pings vs. nur Pull bei PTT?
- [ ] Sleep-Stage-Algorithmus: Cole-Kripke selbst implementieren oder pyActigraphy-Logik portieren?
- [ ] Authentifizierung Gerät <> Backend: Pre-Shared-Key flashen vs. mTLS-Cert?
- [ ] Mehrere Geräte (Schlüsselbund + Schreibtisch + Handgelenk): wie unterscheidet das Backend?

## Spike-Plan

### Spike 0 — Display + Gehäuse (1–2 Wochen)
1. Hardware bestellen ([`BAUTEILE.md`](BAUTEILE.md) Pflicht-Block).
2. Display ansteuern, einfache UI rendern.
3. Watch-Case drucken (existierendes STL), Board einbauen.
4. Battery-Lebensdauer im Idle-Mode messen.

### Spike A — Aktigraphie (1 Woche)
5. IMU-Daten loggen, Schritte zählen.
6. Cole-Kripke-Algo aufsetzen, Tag/Nacht-Erkennung.

### Spike B — PPG / Puls (1–2 Wochen)
7. MAX30102 verlöten + I2C ansprechen.
8. Spot-Check ("Puls drücken") + 5-min-Sampling-Loop.
9. HRV (RMSSD) berechnen.

### Spike C — Sleep-Mode (1 Woche)
10. Power-State, der nachts Display abschaltet, IMU + PPG samplet.
11. Logs in Flash, morgens Sync ans Backend.

### Spike D — Jarvis-Voice-Integration (2–3 Wochen)
12. PTT-Knopf, INMP441 + MAX98357A verkabeln.
13. WiFi-Stream zum Backend (WebSocket + Opus oder PCM).
14. Audio-Wiedergabe der Antwort.

### Spike E — IR-Universal-Remote (1 Woche)
15. IR-LED + Treiber-Transistor.
16. IR-Codes vom Backend pushen lassen.

### Spike F — Mi-Band-Ablöse (laufend)
17. Eine Woche parallel zur Mi Band tragen, Daten vergleichen.
18. Wenn Sleep-Daten plausibel → Mi Band weglegen.
