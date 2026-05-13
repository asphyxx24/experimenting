# Homelab

Kompakter Heimserver auf Basis eines Dell OptiPlex 3090, der gleichzeitig mehrere Rollen übernimmt. Kein Enterprise-Setup, kein Gaming-PC — sondern ein sparsamer, dauerhaft laufender Knotenpunkt für persönliche Dienste, Emulation und Automatisierung.

**Status: Hardware gekauft, in Einrichtung.**

---

## Hardware

**Dell OptiPlex 3090 Micro**
- CPU: Intel Core i5-10500T (6 Kerne / 12 Threads, 3,8 GHz Boost, **35W TDP**)
- RAM: 16 GB DDR4
- Storage: 256 GB SSD (intern)
- OS: Windows 11 (vorinstalliert)

Die T-Variante des i5-10500T ist explizit für Dauerbetrieb ausgelegt — niedrige Leistungsaufnahme (~15–35W unter Last), leise, thermisch stabil.

> **Offene Entscheidung — OS:** Das Gerät läuft aktuell mit Windows 11. Für den geplanten Software-Stack (Docker, Server-Dienste) wäre **Debian 12 oder Ubuntu 24.04 LTS** deutlich einfacher zu betreiben. Docker auf Windows erfordert WSL2 + Docker Desktop und ist komplexer. → Neuinstallation Linux erwägen, oder zumindest WSL2 einrichten.

---

## Rollen

| Rolle | Lösung | Status |
|---|---|---|
| Emulation (bis PS2) | PCSX2, RetroArch | geplant |
| Lokale Cloud | Nextcloud oder Syncthing | geplant |
| DNS-Werbeblocker | AdGuard Home | geplant |
| Heimautomatisierung | Home Assistant | geplant |
| IoT-Endpunkt | Mosquitto MQTT (für ESP32-Projekte) | geplant |
| Workflow-Automatisierung | n8n | geplant |
| Trading Bot | Python-Bot (siehe `ideas/trading-bot`) | geplant |

---

## Emulation

- Ziel: PS1, PS2 und ältere Systeme — **kein** PS3 / Switch
- PCSX2 ist primär CPU-gebunden; Single-Thread-Leistung entscheidend
- i5-10500T schafft ~95% der PS2-Bibliothek problemlos
- Läuft **nativ** (kein Container) für maximale Performance
- Grenzfall: sehr aufwändige PS2-Spiele (Shadow of the Colossus, God of War) — hier testen

---

## Trading Bot

Der Bot (siehe `ideas/trading-bot`) läuft dauerhaft auf dem Homelab — aber nicht 24/7:

- **XETRA / Euronext:** 09:00–17:30 Uhr
- **NASDAQ:** 15:30–22:00 Uhr

Start/Stop via Windows Task Scheduler (oder systemd-Timer bei Linux).

**Ressourcenbedarf:**
- FinBERT-Inferenz auf CPU: ~2 GB RAM, kein GPU nötig
- Bot gesamt: minimal, SQLite wächst langsam
- Externes Reasoning-API (OpenAI/Requesty): kein lokaler RAM-Overhead

---

## Software-Stack

```
┌─────────────────────────────────────┐
│            OptiPlex 3090            │
│                                     │
│  Docker / Podman                    │
│  ├── AdGuard Home      (DNS, :53)   │
│  ├── Nextcloud         (:443)       │
│  ├── Home Assistant    (:8123)      │
│  ├── Mosquitto MQTT    (:1883)      │
│  └── n8n               (:5678)     │
│                                     │
│  Nativ:                             │
│  ├── PCSX2 / RetroArch (Emulation) │
│  └── Trading Bot       (Python)    │
└─────────────────────────────────────┘
```

Alle Server-Dienste in Docker-Containern → einfaches Update, saubere Isolation.

---

## Speicher

256 GB intern werden mittelfristig eng:
- OS + Docker-Volumes: ~30–50 GB
- PS2-ROMs: 1–4 GB pro Spiel
- Nextcloud-Daten: wächst mit der Zeit
- Trading Bot Logs (SQLite): minimal

**Lösung:** Externes Speichermedium hinzufügen
- Option A: USB 3.0 HDD (1–4 TB) — günstig, ausreichend für ROMs + Cloud-Daten
- Option B: 2,5"-SATA-Fach intern (OptiPlex 3090 hat eines) — ordentlicher, gleiche Geschwindigkeit

**Backup:** 3-2-1-Regel — mindestens eine Kopie extern (z.B. Backblaze B2 via `rclone`)

---

## Netzwerk & Remote-Zugriff

- Gigabit LAN bevorzugt (stabiler als WLAN für Server-Dienste)
- Für Remote-Zugriff auf Nextcloud von außen: eigene Domain + DynDNS oder Cloudflare Tunnel
- Alternativ: VPN (WireGuard) ins Heimnetz

---

## MQTT & IoT-Integration

Der Mosquitto MQTT-Broker macht das Homelab zum Knotenpunkt für ESP32-Projekte:
- Companion-Watch (siehe `ideas/companion-watch`) kann Daten an den Broker senden
- Home Assistant empfängt MQTT-Events und reagiert darauf
- Grundlage für weitere Eigenbauten

---

## Stromkosten

| Betrieb | Verbrauch | Kosten/Monat* |
|---|---|---|
| Idle (nur Server-Dienste) | ~10–15 W | ~1,50–2,50 € |
| Last (Bot + Dienste) | ~20–30 W | ~3–5 € |
| Emulation aktiv | ~30–40 W | ~5–7 € |

*Basis: ~0,30 €/kWh

---

## Offene Fragen

- [ ] **OS-Entscheidung**: Windows 11 behalten (WSL2 + Docker Desktop) oder Linux installieren?
- [ ] **Externer Speicher**: USB HDD oder internes 2,5"-Fach?
- [ ] **Remote-Zugriff**: Domain + DynDNS oder Cloudflare Tunnel oder nur VPN?
- [ ] **Welche PS2-Spiele** konkret? → bestimmt ob Grenzfälle relevant sind
- [ ] **Netzwerk**: Gigabit LAN zum Router vorhanden?
