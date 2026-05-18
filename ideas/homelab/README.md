# Homelab

Kompakter Heimserver auf Basis eines Dell OptiPlex 3090, der gleichzeitig mehrere Rollen übernimmt. Kein Enterprise-Setup, kein Gaming-PC — sondern ein sparsamer, dauerhaft laufender Knotenpunkt für persönliche Dienste, Emulation und Automatisierung.

**Status: Hardware gekauft, Linux-Installation ausstehend.**

---

## Hardware

**Dell OptiPlex 3090 Micro**
- CPU: Intel Core i5-10500T (6 Kerne / 12 Threads, 3,8 GHz Boost, **35W TDP**)
- RAM: 16 GB DDR4
- Storage: 256 GB SSD (intern) + 2,5"-SATA-Fach frei für Erweiterung
- OS: Windows 11 (vorinstalliert) → wird ersetzt durch Linux

Die T-Variante des i5-10500T ist explizit für Dauerbetrieb ausgelegt — niedrige Leistungsaufnahme, leise, thermisch stabil.

---

## Betriebssystem: Linux

Windows wird neuinstalliert. Kein einziger geplanter Dienst oder Use Case erfordert Windows — Linux läuft alles davon besser oder gleichwertig.

**Vorteile gegenüber Windows für diesen Stack:**
- Docker läuft nativ (kein WSL2 / Docker Desktop Overhead)
- `systemd` für automatisches Starten von Diensten und Trading Bot
- Kein Windows Update das den Bot oder Dienste unterbricht
- SSH out-of-the-box, weniger RAM-Overhead für das OS selbst
- Alle Server-Dokumentationen (Nextcloud, Home Assistant, AdGuard etc.) primär auf Linux geschrieben

### Basis: Ubuntu 24.04 LTS

Ubuntu 24.04 LTS als Grundlage — stabile, weit verbreitete Basis mit 5 Jahren Support bis 2029. Alle geplanten Tools haben offizielle Ubuntu-Anleitungen.

### Desktop-Umgebung

Die Wahl der DE ist **reine Geschmackssache** — alle drei laufen identisch stabil, haben vollen Software-Zugang und unterscheiden sich bei 16 GB RAM nicht spürbar in der Performance. Einfach ausprobieren was optisch gefällt.

| DE | Ubuntu-Variante | Charakter |
|---|---|---|
| **GNOME** | Ubuntu (Standard) | Modern, aufgeräumt, macOS-ähnlich |
| **KDE Plasma** | Kubuntu | Sehr anpassbar, Windows-ähnlich, modern |
| **XFCE** | Xubuntu | Schlicht, klassisch, ressourcenschonend |

> DEs können parallel installiert werden — beim Login wechseln ist jederzeit möglich. Einfach eines wählen, ausprobieren, ggf. wechseln.

---

## RAM-Auslastung (Worst Case — alles gleichzeitig)

| Was läuft | RAM |
|---|---|
| Desktop (KDE/GNOME/XFCE) | ~400–600 MB |
| Docker (alle Dienste) | ~1,2 GB |
| Trading Bot + FinBERT | ~2,0 GB |
| PCSX2 aktiv | ~1,5 GB |
| IBKR TWS | ~800 MB |
| OS-Kernel + Puffer | ~500 MB |
| **Gesamt** | **~6,5–7 GB** |

16 GB bieten selbst im Worst Case über 50% freien Headroom. Auch `papagei-llm` lokal zu betreiben wäre theoretisch möglich (je nach Modellgröße 4–12 GB zusätzlich).

---

## Rollen

| Rolle | Lösung | Status |
|---|---|---|
| Emulation (bis PS2) | PCSX2, RetroArch | geplant |
| Persönliche Cloud | Nextcloud | geplant |
| DNS-Werbeblocker | AdGuard Home | geplant |
| Heimautomatisierung | Home Assistant | geplant |
| IoT-Endpunkt | Mosquitto MQTT | geplant |
| Workflow-Automatisierung | n8n | geplant |
| Trading Bot | Python-Bot (siehe `ideas/trading-bot`) | geplant |

---

## Software-Stack

```
┌─────────────────────────────────────┐
│            OptiPlex 3090            │
│         Ubuntu 24.04 LTS            │
│                                     │
│  Docker                             │
│  ├── AdGuard Home      (DNS, :53)   │
│  ├── Nextcloud         (:443)       │
│  ├── Home Assistant    (:8123)      │
│  ├── Mosquitto MQTT    (:1883)      │
│  └── n8n               (:5678)     │
│                                     │
│  Nativ (kein Container):            │
│  ├── PCSX2 / RetroArch (Emulation) │
│  ├── IBKR TWS          (Broker)    │
│  └── Trading Bot       (Python)    │
└─────────────────────────────────────┘
```

Alle Server-Dienste in Docker → einfaches Update, saubere Isolation, einheitliche `docker-compose.yml` pro Dienst.

---

## Einrichtungsreihenfolge (Setup-Plan)

1. **Linux installieren** — Ubuntu-Variante wählen, Windows ersetzen
2. **Grundkonfiguration** — SSH aktivieren, Updates, Benutzer einrichten
3. **Docker installieren** — offizielle Ubuntu-Anleitung
4. **AdGuard Home** — zuerst, da DNS-Blocker für alle anderen Geräte im Netz
5. **Nextcloud** — lokale Cloud einrichten, Speicher einbinden
6. **Home Assistant** — Heimautomatisierung, MQTT-Integration
7. **Mosquitto MQTT** — Broker für ESP32-Projekte (Companion-Watch, Auto-Bewässerung)
8. **n8n** — Workflow-Automatisierung
9. **Emulation** — PCSX2 + RetroArch nativ installieren
10. **Trading Bot** — IBKR TWS + Python-Umgebung + systemd-Timer

---

## Emulation

- Ziel: bis PS2 — **kein** PS3 / Switch
- PCSX2 (Flatpak) läuft nativ, kein Container
- i5-10500T schafft ~95% der PS2-Bibliothek problemlos
- Grenzfall: sehr aufwändige PS2-Spiele (Shadow of the Colossus, God of War) — testen

---

## Trading Bot

Läuft nativ als Python-Prozess, gesteuert über `systemd`-Timer:
- **XETRA / Euronext:** 09:00–17:30 Uhr
- **NASDAQ:** 15:30–22:00 Uhr

Kein 24/7-Betrieb nötig — systemd startet und stoppt automatisch zur Marktzeit.

---

## MQTT & IoT-Integration

Mosquitto als zentraler Broker für alle ESP32-Projekte aus diesem Repo:
- `companion-watch` → sendet Gesundheits-/Statusdaten
- `auto-bewaesserung` → sendet Sensor-Daten, empfängt Steuerbefehle
- Home Assistant abonniert alle Topics und reagiert darauf

---

## Speicher

256 GB intern werden mittelfristig eng:

| Was | Größe |
|---|---|
| OS + Desktop | ~15–20 GB |
| Docker Volumes (alle Dienste) | ~20–40 GB |
| PS2-ROMs | 1–4 GB pro Spiel |
| Nextcloud-Daten | wächst (Fotos, Dokumente, Backups) |
| Trading Bot Logs | minimal |

**Erweiterung:** 2,5"-SATA-Fach im OptiPlex 3090 intern verfügbar → **2 TB Samsung 870 EVO (~70 €) empfohlen**, dediziert für Nextcloud-Daten.

**Backup:** 3-2-1-Regel — mindestens eine Kopie extern (Backblaze B2 via `rclone` empfohlen)

---

## Persönliche Cloud (Nextcloud)

Ersetzt Google Drive / iCloud für Fotos, Dokumente, Kontakte, Kalender.

**Scope:** primär persönlicher Gebrauch, maximal 2 weitere Nutzer.

| Feature | Nextcloud-Lösung |
|---|---|
| Fotos (automatischer Sync vom Handy) | Nextcloud Android/iOS App → auto-upload |
| Dokumente / Dateien | WebDAV-Mount oder App |
| Kalender-Sync | CalDAV (Nextcloud Calendar App) |
| Kontakte-Sync | CardDAV (Nextcloud Contacts App) |
| Öffentliche Share-Links | ja — für gelegentliches Teilen mit anderen |

**Zugriff von unterwegs:** über Tailscale (kein offener Port nötig). Nextcloud-App auf Android/iOS unterstützt VPN-Zugriff nativ.

**Storage-Empfehlung:** 2 TB interne SSD für Nextcloud-Volume. Fotos von modernen Handys: ~5 MB/Foto → 2 TB ≈ 400.000 Fotos. Für 1–3 Personen jahrelang ausreichend.

**Externes Backup der Nextcloud-Daten:**
```
rclone sync /nextcloud-data backblaze:mein-bucket --progress
```
Backblaze B2: ~6 $/TB/Monat. Bei 100 GB = ~0,60 $/Monat.

---

## Netzwerk & Remote-Zugriff

Details in [`netzwerk.md`](netzwerk.md) — hier die Kurzfassung:

- **LAN:** Gigabit-Kabel bevorzugt (stabiler als WLAN für Dauerbetrieb)
- **SSH & private Dienste von außen:** Tailscale auf OptiPlex — funktioniert auch hinter CGNAT, kein offener Port nötig
- **Companion Watch / IoT-MQTT von außen:** Hetzner VPS (~4 €/Monat) als Relay + WireGuard-Tunnel zum OptiPlex
- **Cloudflare Tunnel:** nicht empfohlen — 100 MB Upload-Limit, keine UDP-Unterstützung, ToS-Grauzone bei Video

**Vertrag:** beim neuen Anbieter explizit nach "DualStack (echte IPv4, kein CGNAT)" fragen. Checkliste in `netzwerk.md`.
**Router:** FritzBox 7520 ist ein DSL-Router (kein DOCSIS/Kabel). Für Coax-Anschluss: FritzBox 6660 Cable als Phase-1-Ersatz (~80€ gebraucht), oder separates DOCSIS-Modem + OPNsense-Box für Phase 2. Details in `netzwerk.md`.

---

## Stromkosten

| Betrieb | Verbrauch | ~Kosten/Monat |
|---|---|---|
| Idle (nur Server-Dienste) | ~10–15 W | ~1,50–2,50 € |
| Bot aktiv (Marktzeiten) | ~20–30 W | ~3–5 € |
| Emulation aktiv | ~30–40 W | ~5–7 € |

*Basis: ~0,30 €/kWh*

---

## Peripherie

| Gerät | Modell | Status |
|---|---|---|
| Tastatur + Touchpad | Rii Wireless (2.4GHz + Bluetooth) | bestellt |
| Gamepad | offen — z.B. Xbox Controller | wenn Emulation eingerichtet |

### Display-Anschluss

Die Artikelliste zeigt **DisplayPort + VGA** — kein HDMI. Standard-Spezifikation des OptiPlex 3090 Micro hat normalerweise HDMI 1.4 + DisplayPort 1.4. Erst beim Auspacken prüfen.

| Szenario | Lösung | Auflösung |
|---|---|---|
| HDMI vorhanden | direkt ans TV | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort | passiver DP→HDMI Adapter (~10€) | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort, 4K@60Hz gewünscht | aktiver DP 1.4→HDMI 2.0 Adapter (~25€) | 4K@60Hz |

**Für den geplanten Use Case (Netflix, Emulation) ist 1080p@60Hz der Sweet Spot** — PS2-Spiele laufen in niedrigeren Auflösungen, Netflix-Inhalte sind meist 24fps. Der passive Adapter reicht völlig.

### LAN-Adapter

Nativer LAN-Port defekt — USB 3.0 Gigabit Adapter als dauerhafte Lösung (~15€).

Realistische Einbußen: **keine spürbaren.** USB 3.0 Gigabit Adapter erreicht ~900 Mbps, natives LAN ~940 Mbps. Für Nextcloud, AdGuard, Trading Bot und Streaming ist der Unterschied irrelevant — der Heimanschluss ist der echte Flaschenhals, nicht der Adapter.

---

## Offene Fragen

- [ ] **Desktop-Umgebung**: GNOME, KDE Plasma oder XFCE? → ausprobieren
- [ ] **Interner Speicher**: 2 TB Samsung 870 EVO ins 2,5"-SATA-Fach einbauen (Nextcloud-Daten)
- [ ] **Netzwerk**: Gigabit LAN-Kabel zum Router legen (USB 3.0 LAN-Adapter bereits geplant)
- [ ] **Vertrag**: DualStack (echte IPv4) beim neuen Anbieter bestätigen lassen → Checkliste in `netzwerk.md`
- [ ] **Hetzner VPS**: aufsetzen sobald Companion Watch MQTT von außen braucht
