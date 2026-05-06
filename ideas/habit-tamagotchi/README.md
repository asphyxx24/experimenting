# Habit-Tamagotchi

**Status:** 🔬 Recherche
**Letztes Update:** 2026-05-06

## Idee

Ein virtuelles Tier, das von **echten Habits** des Users lebt (Schritte, Wasser, Code-Commits, Schlaf, Lesezeit, ...) und sichtbar darauf reagiert:

- Habits regelmäßig erfüllt → Tier wächst, freut sich, evolved.
- Mehrere Tage geschludert → Tier wird traurig, mager, schläft viel.
- Ganz ignoriert → Tier zieht aus / Winterschlaf (kein "Tod", weniger Frust).

Charme-Hook: anders als Habit-Apps verschwindet das Ding nicht in der Schublade, sondern sitzt sichtbar am Handgelenk und schaut dich an.

**Erste Zielplattform:** [Companion-Watch](../companion-watch/README.md) (ESP32-S3, rundes 240x240-Display). Konzept ist aber nicht an die Hardware gebunden — denkbar auch als Desktop-Widget, Handy-App oder Schreibtisch-Gerät.

## Spielmechanik

- **Stufen:** Egg → Baby → Teen → Adult → Legend. Pro Stufe n Tage Streak.
- **Stats:** Health (Schritte/Sport), Hydration (Wasser, manuell), Brain (Lesezeit/Code-Commits), Sleep (aus Schlaftracking), Mood (Mittel aller Stats).
- **Daily Check:** einmal pro Tag wertet das Backend Habits aus, schickt Update ans Gerät.
- **Evolution / Outfits:** Belohnungs-Sprite pro Wochenstreak.
- **Tod ist raus.** Stattdessen: Tier zieht aus, kommt zurück wenn du wieder dranbleibst.

## Habit-Datenquellen

Daten kommen über ein Backend (z. B. Jarvis) rein, das verschiedene APIs aggregiert:
- **Schritte / Sport:** Health Connect, Mi Fit, Strava
- **Schlaf:** IMU + PPG auf der Watch, oder Health Connect
- **Code-Commits:** GitHub API
- **Wasser / Lesen:** manueller Input (Tap auf der Watch oder App)
- **Kalender / Custom:** Google Calendar, eigene APIs

Pet-Stimmung wird aus den aggregierten Stats berechnet.

## Visuelle Assets

**Pipeline: [Retro Diffusion](https://retrodiffusion.ai)** — AI-Tool, das Pixel-Sprites inkl. Animations-Frames generiert.

Begründung: weder 3D-Modelling noch klassische Pixel-Animation im Skill-Set, Aseprite-Lernkurve nicht im Budget. Bei 240x240-Display ist Pixel-Art ohnehin der natürliche Stil.

**Workflow:**
1. Concept-Sprite (1 Pose) für jede der 5 Stimmungen via Retro Diffusion generieren.
2. Pro Stimmung 2–4 Animations-Frames (Idle-Loop) ableiten.
3. Procedural-Tricks im Code (Y-Bounce, Atem-Skalierung, Augen-Overlay) reduzieren die benötigte Frame-Zahl deutlich.
4. Fallback: einzelne Stimmungen via Fiverr / r/PixelArt nachschärfen, falls AI-Output zwischen Stimmungen inkonsistent wirkt.

**Speicherung auf ESP32:** Sprite-Sheets in PROGMEM (Flash), Format RGB565 oder palettiert.

## Offene Fragen

- [ ] Asset-Pipeline-Details: wie viele Frames pro Stimmung, Stil-Lock (Konsistenz über alle 5 Stimmungen), Sprite-Sheet-Format (RGB565 vs. palettiert)?
- [ ] Welches Tier? Ein festes Design, oder wählbar?
- [ ] Streak-Mechanik: wie viele Tage pro Stufe? Lineare oder exponentielle Progression?
- [ ] Interaktion: Streicheln (Touch), Füttern (Tap), oder rein passiv auf Habit-Daten reagierend?
- [ ] Multi-Plattform: wenn auch Desktop/Handy — wie synchronisieren sich Pet-State und Sprites?

## Nächste Schritte

1. Erstes Concept-Sprite via Retro Diffusion generieren (5 Stimmungen: happy, neutral, sad, sleepy, excited).
2. Idle-Animation auf der [Companion-Watch](../companion-watch/README.md) rendern (Spike 0 der Watch).
3. Spielmechanik-Prototyp: Stats-Berechnung im Backend, Daily Check → Pet-Stimmung.
4. Streak-System + Stufen-Progression implementieren.
