# Ideen-Index

Übersicht aller Ideen in diesem Repo. Status-Legende:

- 🌱 **Brainstorm** — erste Idee, noch nicht durchdacht
- 🔬 **Recherche** — wird gerade evaluiert / Machbarkeit geprüft
- 🛠 **Prototyp** — Code/Build im Gange
- 🚀 **Aktiv** — wird ernsthaft weiterentwickelt
- ❄️ **Eingefroren** — pausiert, ggf. später wieder aufgreifen
- ⚰️ **Verworfen** — bewusst nicht weiterverfolgt (Begründung in der README)

## Heim-Stack

Diese drei Ideen bauen aufeinander auf: der Homelab-Server hostet Jarvis, Jarvis ist das Backend der Companion-Watch, die Watch schickt ihre Daten über MQTT zurück ins Homelab. Änderungen an einer betreffen oft die anderen — bei Architektur-Entscheidungen die Nachbarn mitlesen.

| Idee | Status | Kurzbeschreibung |
|------|--------|------------------|
| [homelab](homelab/README.md) | 🛠 Prototyp | Dell OptiPlex 3090 als Heimserver: Emulation, Nextcloud, Jellyfin, MQTT, eigene Heimautomatisierung (kein Home Assistant) (Trading Bot: optional/zurückgestellt) |
| [companion-watch](companion-watch/README.md) | 🔬 Recherche | Selbstbau-Smartwatch (ESP32-S3): Jarvis-Voice-PTT, IR-Remote, Health-Tracking, Habit-Tamagotchi-Pet-UI — Mi-Band-Ersatz |
| [jarvis](jarvis/README.md) | 🔬 Recherche | Sprachassistent auf dem Homelab-Server: Push-to-Talk → whisper.cpp → Claude (Headless) → TTS. Backend für die Companion-Watch. Latenz-Messung erledigt (~2,1 s pro Antwort) |

## Eigenständig

| Idee | Status | Kurzbeschreibung |
|------|--------|------------------|
| [trading-bot](trading-bot/README.md) | 🔬 Recherche | Modularer Aktien-Trading-Bot (IBKR, FinBERT Sentiment, vectorbt Backtesting) |
| [piano-display](piano-display/README.md) | 🔬 Recherche | Kabelloser 13" RPi-5-Touchscreen fürs E-Piano — Noten, YouTube, optional PC-Zusatzmonitor |
| [papagei-llm](papagei-llm/README.md) | 🔬 Recherche | Eigenes LLM für Verkaufstelefonie (Outbound/Inbound), self-hosted für niedrige Latenzen |
| [minecraft-controller](minecraft-controller/README.md) | 🔬 Recherche | Controller-Support für Minecraft/Hypixel Skyblock am Mini-PC — Controlify statt Controllable, Scuf-Paddle-Mapping |

<!-- Neue Ideen in die passende Gruppe eintragen. Reihenfolge innerhalb einer Gruppe: aktivste oben. -->
