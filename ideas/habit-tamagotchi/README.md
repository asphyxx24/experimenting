# Habit-Tamagotchi

**Status:** 🌱 Brainstorm
**Letztes Update:** 2026-04-29

## Idee

Ein kleines E-Ink-Gerät am Schlüsselbund / auf dem Schreibtisch, das ein virtuelles Tier zeigt. Das Tier lebt von **echten Habits** des Users (Schritte, Wasser, Code-Commits, Lesezeit, …) und reagiert sichtbar darauf:

- Habits regelmäßig erfüllt → Tier wächst, freut sich, "evolved" in Stufen.
- Mehrere Tage geschludert → Tier wird traurig, mager, schläft viel.
- Ganz ignoriert → Tier zieht aus / geht in Winterschlaf (kein "Tod" — Frust statt Spaß).

Charme-Hook: anders als Habit-Apps, die in der Schublade verschwinden, hängt das Ding sichtbar am Schlüsselbund und schaut dich an.

## Use Case

- Morgens beim Rausgehen: kurzer Blick auf Schlüssel → "Mist, ich war gestern faul, das Vieh schaut traurig."
- Abends: Schritte erreicht → Animation "Tier macht Liegestütze" für ein paar Sekunden via Partial Refresh.
- Wochenstreak → Evolution / neuer Sticker / neues Outfit.

## Hardware-Optionen

| Option | Display | MCU | Größe | Akku-Laufzeit | Preis |
|--------|---------|-----|-------|---------------|-------|
| **Waveshare 2.13" + ESP32-C3** | 250×122 SW | ESP32-C3 | Klein, aber nicht Keychain-tauglich | Wochen mit Deep Sleep | ~25 € |
| **LilyGO T5 / TTGO T5** | 2.13" / 2.9" SW | ESP32 + integrierter E-Ink-Driver | All-in-One Board | Wochen | ~30 € |
| **Inkplate 2** | 2.13" SW | ESP32, fertiges Gehäuse-Konzept | Tischgröße | Monate | ~50 € |
| **M5Stack CoreInk** | 1.54" SW | ESP32 | Sehr klein, fertig im Gehäuse | Wochen | ~30 € |
| **nRF52840 + 1.54" Waveshare** | 152×152 SW | nRF52840 (BLE only) | Sehr stromsparend | Monate–Jahr | ~25 € |
| **Pimoroni Badger 2040 W** | 2.9" SW | RP2040 + WiFi | Anhänger-tauglich | Tage–Wochen | ~30 € |

**Tendenz:** für Keychain-Variante → **M5Stack CoreInk** oder **nRF52 + 1.54"-Waveshare**. Für Schreibtisch-Variante → **Inkplate 2** wegen fertigem Look.

## Architektur (Skizze)

```
[Phone-App / Backend]  ─BLE/WiFi─►  [E-Ink-Device]
        ▲                                 │
        │                                 │ Render (Partial Refresh)
        │                                 ▼
[Habit-Quellen]                      [Display: Tier + Status]
- Apple Health / Google Fit
- Strava / Garmin
- GitHub Commits
- Custom API (Wasser, Lesezeit, …)
- Manuelle Taps (Knopf am Gerät)
```

Zwei Ansätze:

1. **Phone-zentriert (BLE-Sync, alle paar Stunden):** Logik & Habit-Aggregation läuft auf dem Handy, Device ist nur Display. Vorteil: lange Akkulaufzeit, einfache Firmware. Nachteil: braucht Companion-App.
2. **Cloud-zentriert (WiFi):** Device pollt selbst ein Backend. Vorteil: keine App nötig. Nachteil: Stromhungriger, Setup pro WLAN nötig.

→ **Variante 1 ist vermutlich die richtige.**

## Spielmechanik

- **Stufen:** Egg → Baby → Teen → Adult → Legend. Jede Stufe braucht n Tage Streak.
- **Stats:** Health (Schritte / Sport), Hydration (Wasser), Brain (Lesezeit / Code), Mood (Mittel über alles).
- **Daily Check:** einmal pro Tag (z.B. 22:00) wertet das Phone aus, ob Habits erfüllt sind, und schickt Update ans Device.
- **Evolution / Outfits:** Belohnungs-Sprite pro Wochenstreak. Charm-Faktor.
- **Tod ist raus.** Stattdessen: "Tier zieht aus, kommt zurück wenn du wieder dranbleibst." Weniger frustig, weniger drama-trigger.

## Offene Fragen

- [ ] Form-Factor entschieden? Keychain (klein, robust, < 2") vs. Schreibtisch (4"+ mit Detail)?
- [ ] Welche Habit-Quellen wirklich? Manuelle Taps reichen vielleicht schon (1 Knopf = 1 Habit-Done für heute).
- [ ] Companion-App: native Android-only (passt zu vorhandenen Geräten)? Oder Flutter / React Native für beide?
- [ ] Wie viele Sprites muss man malen, damit das Tier "lebendig" wirkt? 20? 100?
- [ ] Gibt es schon was Ähnliches Open Source, das man forken kann (Pebble-Watchfaces? Flipper-Zero-Pets?)?
- [ ] Sound? Vibration? Oder bewusst stumm/passiv (Schlüsselbund-tauglich)?
- [ ] Privacy: Health-Daten bleiben besser nur auf dem Phone — keine Cloud.

## Recherche-Ideen vor dem Bau

- Bestehende Projekte sichten: "M5Stack Tamagotchi", "ESP32 e-ink pet", "Watchy pet face", "Flipper Zero Tamagotchi App".
- E-Ink Partial Refresh: wie viele Frames/Sekunde realistisch? (Typisch 5–10 Hz partial, > 1s full).
- Pixel-Art-Stil definieren — passt zum SW-Display und ist machbar ohne Profi-Designer.

## Nächste Schritte

### Phase 1 — Form-Factor + Plattform entscheiden (1 Woche)
1. Vergleich Keychain vs. Schreibtisch → wonach hätten *wir* mehr Bock?
2. Hardware-Auswahl basierend darauf (siehe Tabelle oben).
3. Existierende Open-Source-Projekte als Basis suchen.

### Phase 2 — Minimal-Prototyp (2–3 Wochen)
4. Display ansteuern, ein Sprite anzeigen.
5. Drei Stimmungen (happy / neutral / sad) statisch durchschalten via Knopf.
6. Battery-Lebensdauer messen.

### Phase 3 — Habit-Sync (2–4 Wochen)
7. Companion-App-Stub (Android), liest Google Fit Schritte.
8. BLE-Pairing + tägliches Sync-Protokoll.
9. Stimmung des Tiers richtet sich nach echtem Schritt-Wert.

### Phase 4 — Polish + Spielmechanik (offen)
10. Mehr Habit-Quellen anbinden.
11. Sprite-Set ausarbeiten, Evolutionen.
12. Gehäuse / 3D-Druck.
