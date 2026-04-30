# Habit-Tamagotchi → Companion-Device

**Status:** 🔬 Recherche
**Letztes Update:** 2026-04-29

## Idee

Ein kleines Gerät als Smartwatch (oder optional Schlüsselbund/Schreibtisch), das ein virtuelles Tier zeigt. Das Tier lebt von **echten Habits** des Users (Schritte, Wasser, Code-Commits, Schlaf, Lesezeit, …) und reagiert sichtbar darauf:

- Habits regelmäßig erfüllt → Tier wächst, freut sich, evolved.
- Mehrere Tage geschludert → Tier wird traurig, mager, schläft viel.
- Ganz ignoriert → Tier zieht aus / Winterschlaf (kein "Tod", weniger Frust).

Charme-Hook: anders als Habit-Apps verschwindet das Ding nicht in der Schublade, sondern sitzt sichtbar am Handgelenk und schaut dich an.

## Wesentliche Entscheidung: Companion statt Standalone

Diese Idee wird **als Spike 0 vom [Jarvis-Companion-Device](../../.context/attachments/pasted_text_2026-04-29_14-50-06.txt) gebaut**, nicht als isoliertes Tamagotchi. Begründung:

- Selbe Hardware-Klasse (ESP32-S3, rundes Display, Akku, Sensoren).
- Selbes Form-Factor-Ziel (Smartwatch).
- Pet-Persönlichkeit ist gleichzeitig die Repräsentation von Jarvis (eine Stimmung, ein Wesen, mehrere Modi) — kein Zoo.
- Habit-Daten kommen über das Jarvis-Backend rein (Mi-Fit / Google-Fit / Health Connect), Pet-Stimmung folgt daraus.

## Use Case

- 24/7 am Handgelenk getragen → ersetzt Xiaomi Mi Band komplett (siehe Trade-off unten).
- Morgens Blick aufs Handgelenk → Pet-Stimmung = Status der gestrigen Habits.
- Tagsüber Push-to-Talk-Knopf für Jarvis-Voice-Anfragen.
- IR-Funktion als Universal-Remote (TV/HiFi/Klima vom Handgelenk).
- Nachts Schlaftracking via IMU + PPG.

## Hardware-Entscheidung

**Gewählt: Waveshare ESP32-S3-Touch-LCD-1.28** (~25 €).

Upgrade gegenüber dem non-Touch-Modell (~20 €): kapazitiver Touchscreen on-board — nützlich für Streicheln/Wischen im Tamagotchi-UI, kaum Aufpreis, alles andere identisch.

Begründung gegen die Alternativen:
- Gegen E-Ink-Varianten: zu langsam für Sprite-Animation, kein Charme.
- Gegen Sharp Memory LCD: kleinere Auswahl an fertigen Cases.
- Gegen RP2040-Variante: kein WiFi/BLE — Killer für Voice + Sync.
- Gegen LilyGo T-Watch-S3: teurer (~60 €), weniger flexibel beim Bauteil-Layout.
- Gegen ESP32-S3-LCD-1.85: rund, mehr Pixel (360×360), aber zu groß für eine Uhr.
- Gegen ESP32-S3-Touch-AMOLED-2.06: hat Mikro + Speaker on-board (gut), aber fast keine freien GPIOs mehr — IR, Vibration, RGB-LED kaum unterzubringen. Außerdem rechteckig.
- Gegen ESP32-C3/C6-Varianten: single-core, zu schwach für flüssige Animation + Voice-Streaming gleichzeitig.

ESP32-S3 hat WiFi + BLE 5.0, 240 MHz Dual-Core, IMU on-board, USB-C-Charging, 65k-Farben-IPS-Display. Animation mit 30–60 fps problemlos.

**Komplette Bauteile-Liste:** siehe [`BAUTEILE.md`](BAUTEILE.md).

## Spielmechanik

- **Stufen:** Egg → Baby → Teen → Adult → Legend. Pro Stufe n Tage Streak.
- **Stats:** Health (Schritte/Sport), Hydration (Wasser, manuell), Brain (Lesezeit/Code-Commits), Sleep (aus PPG+IMU), Mood (Mittel).
- **Daily Check:** einmal pro Tag wertet das Backend Habits aus, schickt Update via WiFi ans Gerät.
- **Evolution / Outfits:** Belohnungs-Sprite pro Wochenstreak.
- **Tod ist raus.** Stattdessen: Tier zieht aus, kommt zurück wenn du wieder dranbleibst.
- **Pet = Jarvis-Avatar:** Stimmung reflektiert auch Backend-State ("denkt nach" = LLM-Call läuft, "schläft" = Backend offline).

## Visuelle Assets

**Pipeline gewählt: [Retro Diffusion](https://retrodiffusion.ai)** — AI-Tool, das Pixel-Sprites inkl. Animations-Frames direkt für genau solche Use-Cases generiert.

Begründung: weder 3D-Modelling noch klassische Pixel-Animation im Skill-Set, Aseprite-Lernkurve nicht im Budget. Bei 240×240-Display ist Pixel-Art ohnehin der natürliche Stil.

**Workflow:**
1. Concept-Sprite (1 Pose) für jede der 5 Stimmungen via Retro Diffusion generieren.
2. Pro Stimmung 2–4 Animations-Frames (Idle-Loop) ableiten.
3. Procedural-Tricks im Code (Y-Bounce, Atem-Skalierung, Augen-Overlay) reduzieren die benötigte Frame-Zahl deutlich.
4. Fallback: einzelne Stimmungen via Fiverr / r/PixelArt nachschärfen, falls AI-Output zwischen Stimmungen inkonsistent wirkt.

**Speicherung auf dem ESP:** Sprite-Sheets in PROGMEM (Flash), Format RGB565 oder palettiert.

## Mi-Band-Ersatz — Trade-offs

| | Mi Band 8 | Companion | Bewertung |
|--|-----------|-----------|-----------|
| Akku | 14 Tage | 2–3 Tage | Mi Band gewinnt klar |
| Schritte | ✓ | ✓ (IMU) | gleich |
| Puls 24/7 | ✓ | ✓ (MAX30102) | gleich |
| Schlaf-Stages | ✓ | ✓ (Algo-Port nötig) | gleich |
| SpO2 | ✓ | ✓ | gleich |
| Voice/Jarvis-PTT | ✗ | ✓ | unser Vorteil |
| IR-Remote | ✗ | ✓ | unser Vorteil |
| Pet-Personality | ✗ | ✓ | unser Vorteil |
| Tragekomfort | super dünn | klotzig (~18 mm) | Mi Band gewinnt |
| Daten offen / hackbar | proprietär | komplett offen | unser Vorteil |

**Konsequenz:** ersetzt die Mi Band, wenn man die Lade-Routine alle 2–3 Tage und die Bauhöhe akzeptiert. Algorithmen für Sleep-Staging müssen portiert werden (z. B. Cole-Kripke, [pyActigraphy](https://github.com/ghammad/pyActigraphy), [Bangle.js Sleep-Logger](https://github.com/espruino/BangleApps/tree/master/apps/sleeplog)).

## Architektur (Skizze)

```
[Jarvis-Backend] ──WiFi/WebSocket──► [ESP32-S3 Companion am Handgelenk]
        ▲                                       │
        │                                       │ Display: Pet + Status
        │ Datenquellen + Push                   │ Audio I2S in/out
        │                                       │ IR-Blaster
        ▼                                       │ IMU + PPG
[Health Connect / Mi Fit / Strava /             │
 GitHub / Calendar / Custom-API]                │
                                                ▼
                                       [User: Tap, PTT, Schlaf, …]
```

Das Gerät ist **dummer Client mit IO**, nicht eigenständige AI. Modi:
1. **Idle:** Pet-Animation, IMU lauscht, WiFi off.
2. **Voice:** PTT gedrückt → WiFi connect → Audio-Stream zu Jarvis → Antwort.
3. **Remote:** Display zeigt Buttons, IR/BLE/WiFi-Calls feuern.
4. **Notification:** Backend pusht → Display + ggf. Vibration.
5. **Sleep:** Display off, IMU + PPG samplen, alles in Flash, morgens syncen.

## Offene Fragen

- [ ] Watch-Case: bestehendes STL ([Watch-Case mit Akku-Slot](https://www.printables.com/model/484236-waveshare-rp2040-mcu-board-with-128inch-round-lcd-)) reicht erstmal, oder direkt eigener Custom-Print?
- [ ] PTT-Hardware: Side-Push am Gehäuse vs. Touchscreen-Hold vs. IMU-Geste? → Knopf am verlässlichsten.
- [ ] Backend-Push-Protokoll: WebSocket dauerhaft offen vs. MQTT mit Wakeup-Pings vs. nur Pull bei PTT?
- [ ] Sleep-Stage-Algorithmus: Cole-Kripke selbst implementieren oder pyActigraphy-Logik portieren?
- [ ] Authentifizierung Gerät ↔ Backend: Pre-Shared-Key flashen vs. mTLS-Cert?
- [ ] Mehrere Geräte (Schlüsselbund + Schreibtisch + Handgelenk): wie unterscheidet das Backend?
- [ ] Asset-Pipeline-Details: wie viele Frames pro Stimmung, Stil-Lock (Konsistenz über alle 5 Stimmungen), Sprite-Sheet-Format (RGB565 vs. palettiert)?

## Nächste Schritte

### Spike 0 — Tamagotchi-Idle-Loop (1–2 Wochen)
1. Hardware bestellen ([`BAUTEILE.md`](BAUTEILE.md) Pflicht-Block).
2. Display ansteuern, ein Sprite, drei Stimmungen statisch.
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
