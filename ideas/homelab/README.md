# Homelab

Kompakter Heimserver auf Basis eines Dell OptiPlex 3090, der gleichzeitig mehrere Rollen übernimmt. Kein Enterprise-Setup, kein Gaming-PC — sondern ein sparsamer, dauerhaft laufender Knotenpunkt für persönliche Dienste, Emulation, Automatisierung und Heimstreaming.

**Status: Hardware gekauft, Linux-Installation ausstehend.**

---

## Hardware

**Dell OptiPlex 3090 Micro**
- CPU: Intel Core i5-10500T (6 Kerne / 12 Threads, 3,8 GHz Boost, **35W TDP**)
- iGPU: Intel UHD 630 — unterstützt Intel Quick Sync (H.264 + H.265/HEVC hardware-beschleunigt)
- RAM: 16 GB DDR4
- Storage: 256 GB SSD (intern, M.2) + 2,5"-SATA-Fach frei für Erweiterung
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
| Docker (alle Dienste inkl. Jellyfin) | ~1,5 GB |
| Trading Bot + FinBERT | ~2,0 GB |
| PCSX2 aktiv | ~1,5 GB |
| IBKR TWS | ~800 MB |
| OS-Kernel + Puffer | ~500 MB |
| **Gesamt** | **~7–8 GB** |

16 GB bieten selbst im Worst Case über 50% freien Headroom. Auch `papagei-llm` lokal zu betreiben wäre theoretisch möglich (je nach Modellgröße 4–12 GB zusätzlich).

---

## Rollen

| Rolle | Lösung | Status |
|---|---|---|
| Emulation (bis PS2) | PCSX2, RetroArch | geplant |
| Lokale Cloud / Dateisync | Nextcloud | geplant |
| NAS / Netzlaufwerk | Samba | geplant |
| Medienserver / Heimstreaming | Jellyfin | geplant |
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
│  ├── Jellyfin          (:8096)      │
│  ├── Samba             (:445)       │
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

## NAS & Heimstreaming

### Konzept

Jellyfin läuft als Medienserver im Docker. Filme und Serien liegen auf der externen HDD, Jellyfin indexiert alles automatisch mit Postern, Metadaten und Untertiteln. Streaming auf beliebige Geräte im Heimnetz — oder über Tailscale von unterwegs. Samba macht denselben Speicher als klassisches Netzlaufwerk zugänglich.

### Video-Format: H.265 (HEVC) — immer

H.265 spart ~50% Speicher gegenüber H.264 bei gleicher Qualität. Praktisch alle relevanten Abspielgeräte der letzten ~10 Jahre unterstützen es nativ.

**Dateigrößen (H.265, 1080p):**
| Inhalt | Größe |
|---|---|
| Spielfilm (2h) | ~2–6 GB |
| Serienstaffel (10 × 45 min) | ~5–15 GB |
| Planungswert | ~3 GB/Film |
| **4 TB Festplatte** | **~1.000–1.300 Filme** |

**H.265-Kompatibilität moderner Geräte:**
| Gerät | H.265 ab |
|---|---|
| iPhone | iPhone 6 (2014) |
| Android | ~2015 |
| MacBook | 2015 |
| Windows-Laptop | ~2016 (Intel 6. Gen) |
| Smart TV | ~2016 |

Geräte der letzten ~10 Jahre können H.265 nativ — Transcoding ist die Ausnahme.

### Direct Play vs. Transcoding

| Modus | Was passiert | CPU-Last |
|---|---|---|
| **Direct Play** | Abspielgerät spielt 1:1 ab | ~0% |
| **Transcoding** | Jellyfin konvertiert on-the-fly | mittel–hoch |

Direct Play ist der Normalfall. Transcoding passiert nur wenn das Gerät den Codec nicht kann oder die Bitrate zu hoch ist.

**Transcoding-Kapazität des i5-10500T:**
| Modus | Kapazität |
|---|---|
| Hardware-Transcoding (Quick Sync via VAAPI) | ~3–4 parallele 1080p-Streams |
| Software-Transcoding (CPU) | ~2–3 parallele 1080p-Streams |
| Direct Play | praktisch unbegrenzt |

### Jellyfin: Quick Sync auf Linux einrichten

1. Paket installieren: `sudo apt install intel-media-va-driver`
2. In `docker-compose.yml` das DRI-Gerät durchreichen:
   ```yaml
   devices:
     - /dev/dri:/dev/dri
   ```
3. In Jellyfin: Dashboard → Wiedergabe → Transkodierung → **Intel QuickSync** aktivieren

> Ohne Schritt 2 sieht der Container die iGPU nicht — kein Hardware-Transcoding, alles läuft dann auf der CPU.

### Medien-Quellen: Blu-rays rippen

Blu-rays und DVDs kaufen und rippen:
- **MakeMKV** — ripped die Disc zur MKV-Datei (verlustfrei, große Datei)
- **Handbrake** — komprimiert auf H.265 1080p (empfohlen für Speichereffizienz)
- Externes USB Blu-ray-Laufwerk nötig (~60–80€) — auf MakeMKV-Kompatibilitätsliste prüfen
- Für **4K UHD Blu-rays**: nur bestimmte Laufwerke (ggf. Firmware-Modifikation nötig), deutlich aufwändiger

> **Rechtlicher Hinweis (Deutschland):** § 95a UrhG verbietet das Umgehen technischer Schutzmaßnahmen (AACS-Verschlüsselung auf Blu-rays) — auch für Privatkopien. Das Rippen kommerzieller Blu-rays ist nach aktuellem deutschen Recht rechtlich problematisch, auch wenn es in der Praxis nicht verfolgt wird.

---

## Einrichtungsreihenfolge (Setup-Plan)

1. **Linux installieren** — Ubuntu-Variante wählen, Windows ersetzen
2. **Grundkonfiguration** — SSH aktivieren, Updates, Benutzer einrichten
3. **Docker installieren** — offizielle Ubuntu-Anleitung
4. **`intel-media-va-driver` installieren** — für Quick Sync / VAAPI
5. **AdGuard Home** — zuerst, da DNS-Blocker für alle anderen Geräte im Netz
6. **Jellyfin** — Medienserver, VAAPI konfigurieren, externe HDD einbinden
7. **Samba** — Netzlaufwerk einrichten
8. **Nextcloud** — lokale Cloud einrichten, Speicher einbinden
9. **Home Assistant** — Heimautomatisierung, MQTT-Integration
10. **Mosquitto MQTT** — Broker für ESP32-Projekte (Companion-Watch, Auto-Bewässerung)
11. **n8n** — Workflow-Automatisierung
12. **Tailscale** — Remote-Zugriff einrichten
13. **Emulation** — PCSX2 + RetroArch nativ installieren
14. **Trading Bot** — IBKR TWS + Python-Umgebung + systemd-Timer

---

## Emulation

- Ziel: bis PS2 — **kein** PS3 / Switch
- PCSX2 (Flatpak) läuft nativ, kein Container
- i5-10500T schafft ~95% der PS2-Bibliothek problemlos
- Grenzfall: sehr aufwändige PS2-Spiele (Shadow of the Colossus, God of War) — testen

### Emulatoren nach System

| System | Primär | Alternative / Hinweis |
|---|---|---|
| NES | RetroArch: **Mesen** | FCEUmm als Fallback |
| SNES | RetroArch: **bsnes** | Snes9x (leichter) |
| Game Boy / GBC | RetroArch: **Gambatte** | SameBoy für besseres Farbhandling |
| GBA | RetroArch: **mGBA** | Aktiv entwickelt, beste Genauigkeit |
| Mega Drive / Genesis | RetroArch: **Genesis Plus GX** | Deckt auch Master System + Game Gear ab |
| N64 | RetroArch: **Mupen64Plus-Next** | simple64 (standalone, leichter) |
| Saturn | RetroArch: **Beetle Saturn** | BIOS Pflicht; sehr CPU-intensiv |
| Dreamcast | RetroArch: **Flycast** | Redream (standalone) für einfacheres UI |
| PS1 | **DuckStation** (standalone) | Beetle PSX HW im RetroArch-Core |
| PS2 | **PCSX2** (standalone Flatpak) | — |

> PS1, PS2 und Saturn benötigen BIOS-Dateien — von eigener Hardware dumpen.

### Controller

Beide Varianten kabellos (2.4GHz-Dongle + Bluetooth), Plug-and-Play unter Ubuntu 24.04 (Kernel 6.8+, kein Extra-Treiber), SDL-kompatibel → direkt in RetroArch & PCSX2 erkannt.

| Controller | Preis | Layout | Für |
|---|---|---|---|
| **8BitDo Pro 2** | ~45–50 € | SNES-Style | Wer klassisches Button-Layout mag — ideal für 2D-Klassiker |
| **8BitDo Ultimate 2** | ~55–60 € | Xbox-Style | Ergonomischer für lange Sessions mit Analog-Sticks (N64/PS1/PS2) |

Beide haben exzellentes D-Pad, Analog-Sticks und funktionieren für alle Systeme im Setup.

### Adapter für Original-Controller (optional)

Wer mit originalen Controllern spielen möchte — **Raphnet-Tech** als Qualitätsstandard (USB-HID, kein Treiber nötig):

| Adapter | Preis (USD) | Für |
|---|---|---|
| SNES/NES to USB | ~16 | Originale SNES/NES-Controller |
| GC/N64 to USB | ~16 | N64-Controller (präzisere Analog-Achse als Mayflash) |
| PS1/PS2 to USB | ~29 | Originale PS1/PS2-Controller |

Versand aus Kanada (~1–2 Wochen). Günstigere Alternative: **Mayflash** (~10–15 € Amazon) — für Gelegenheitsnutzung OK, N64-Analog etwas ungenauer.

> Für den Einstieg reicht ein moderner Controller (8BitDo oder Xbox). Raphnet-Adapter erst kaufen wenn Original-Controller-Feeling gewünscht.

### PCSX2 Setup (Linux)

```bash
flatpak install flathub net.pcsx2.PCSX2
```

- Input-Backend: **SDL** (Settings → Controllers → Input Source)
- BIOS: in `~/.config/PCSX2/bios/` ablegen
- Renderer: **Vulkan** bevorzugt (besser als OpenGL für Upscaling & Performance)

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

**Saubere Aufteilung nach Funktion:**

| Speicher | Verwendung |
|---|---|
| 256 GB M.2 SSD (intern) | Ubuntu + Docker-Volumes + alle Dienste |
| **1–2 TB 2,5" HDD intern** | ROMs (alle Systeme) + Nextcloud-Daten |
| **4 TB USB 3.0 HDD extern** | Filmbibliothek (Jellyfin) |
| USB-Stick | Wirklich unersetzliche Dateien (Passwörter, wichtige Dokumente) |

> **Warum 2,5"-SATA für ROMs und nicht für Filme?** 2,5"-HDDs sind auf max. 2 TB (CMR) limitiert — zu klein für die Filmbibliothek. Für ROMs reicht 1 TB locker. 4-TB-Modelle in 2,5" gibt es nur mit SMR und 15mm Bauhöhe, die nicht in den OptiPlex-Schacht passt.

### ROMs auf HDD: kein Geschwindigkeitsproblem

Emulatoren laden Spiele beim Start komplett in den RAM. Die HDD-Geschwindigkeit betrifft nur die Ladezeit, nicht das Gameplay.

Zum Vergleich: die originale PS2-DVD las mit maximal ~5,4 MB/s — eine 2,5"-HDD liest mit ~80–120 MB/s, also 15–20× schneller. Es gibt keine Emulations-Generation, bei der HDD-Geschwindigkeit ein Problem wäre.

**Größenabschätzung ROMs (persönliche Sammlung):**

| System | Größe pro Spiel | 50 Spiele |
|---|---|---|
| NES / SNES / GBA | < 10 MB | < 1 GB |
| N64 | 8–64 MB | ~2 GB |
| Saturn / Dreamcast | 700 MB – 1 GB | ~50 GB |
| PS1 | 600–700 MB | ~35 GB |
| PS2 | 2–8 GB | ~200–400 GB |

Eine **1 TB interne HDD** reicht für eine persönliche Sammlung über alle Systeme. Mit 2 TB ist man auf lange Zeit sicher.

### Filmbibliothek auf externer USB HDD: kein Geschwindigkeitsproblem

Die USB 3.0-Verbindung ist nicht der Flaschenhals — die HDD selbst ist es, und sie ist trotzdem mehr als schnell genug:

| | Geschwindigkeit |
|---|---|
| 3,5" HDD Lesegeschwindigkeit | ~120–180 MB/s |
| 1080p H.265 Stream | ~5–15 MB/s |
| 3 gleichzeitige Streams | ~45 MB/s → HDD schafft das 3–4× über |

Einziger USB-Hinweis: USB-Autosuspend für Speicher in Ubuntu deaktivieren, damit die Platte sich nicht unerwartet trennt.

**HDD-Parken deaktivieren** (wichtig für 24/7-Betrieb):
```bash
hdparm -S 0 /dev/sdX
```
Ohne diese Einstellung akkumuliert die HDD tausende unnötige Start/Stop-Zyklen pro Jahr.

### Backup-Strategie

Keine zweite identische HDD nötig — die Filmbibliothek kann im Worst Case neu gerippt werden. Stattdessen:

| Was | Backup wie |
|---|---|
| Filmbibliothek (4 TB extern) | kein separates Backup — neu rippbar |
| ROMs + Nextcloud-Daten (intern) | USB-Stick als Kopie der wirklich wichtigen Sachen |
| Dokumente / Passwörter | USB-Stick, räumlich getrennt aufbewahren |

Wer doch ein vollständiges Backup will: zweite externe HDD + automatisches `rsync` per systemd-Timer reicht vollkommen — keine Cloud nötig.

---

## Netzwerk & Remote-Zugriff

- **LAN:** Gigabit-Kabel bevorzugt — nativer LAN-Port defekt, USB 3.0 Gigabit Adapter (~15€) als Dauerlösung. Realer Durchsatz ~700–900 Mbps, für Streaming und alle Dienste völlig ausreichend.
- **SSH:** direkt im lokalen Netz, kein Extra-Setup nötig

### Remote-Zugriff: Tailscale (empfohlen)

Tailscale erstellt ein privates WireGuard-Mesh-Netzwerk zwischen eigenen Geräten. Kein Port-Forwarding, funktioniert hinter CGNAT, keine feste IP nötig.

- Installation auf OptiPlex + alle Client-Geräte (Handy, Laptop)
- Jedes Gerät bekommt eine stabile interne IP (`100.x.x.x`)
- Jellyfin, Nextcloud etc. erreichbar als wären sie im Heimnetz
- Kostenlos bis 100 Geräte / 3 Nutzer

> Tailscale erfordert die App auf jedem Client-Gerät. Wer Dienste ohne App für andere teilen möchte → Cloudflare Tunnel als Alternative.

### Streaming von außerhalb — Upload-Bedarf

| Qualität | Upload pro Stream |
|---|---|
| 1080p H.265 | ~5–15 Mbit/s |
| 1080p H.264 | ~8–25 Mbit/s |

Im Heimnetz: kein Internet-Upload nötig — alles läuft lokal.

---

## Stromkosten

| Betrieb | Verbrauch | ~Kosten/Monat |
|---|---|---|
| Idle (nur Server-Dienste) | ~10–15 W | ~1,50–2,50 € |
| Bot aktiv (Marktzeiten) | ~20–30 W | ~3–5 € |
| Emulation aktiv | ~30–40 W | ~5–7 € |

*Basis: ~0,30 €/kWh*

---

## Peripherie & Einkaufsliste

### Bereits bestellt / vorhanden

| Gerät | Modell | Status |
|---|---|---|
| Tastatur + Touchpad | Rii Wireless (2.4GHz + Bluetooth) | bestellt |

### Noch zu kaufen

| Was | Warum | ~Kosten |
|---|---|---|
| USB 3.0 Gigabit LAN-Adapter | Nativer Port defekt | ~15 € |
| **2 TB 2,5" HDD intern** (WD Blue WD20SPZX oder Seagate ST2000LM015 — beide 7mm CMR) | ROMs + Nextcloud-Daten | ~40–50 € |
| **4 TB USB 3.0 HDD extern** | Filmbibliothek (Jellyfin) | ~80–100 € |
| **Externes USB Blu-ray-Laufwerk** | Discs rippen mit MakeMKV | ~60–80 € |
| USB-Stick (32–64 GB) | Backup wichtiger Dateien | ~10–15 € |
| **Gamepad** — 8BitDo Pro 2 (SNES-Layout, 2D-Fokus) oder Ultimate 2 (Xbox-Layout, Analog-Fokus) | Emulation | ~45–60 € |

### Display-Anschluss

Die Artikelliste zeigt **DisplayPort + VGA** — kein HDMI. Standard-Spezifikation des OptiPlex 3090 Micro hat normalerweise HDMI 1.4 + DisplayPort 1.4. Erst beim Auspacken prüfen.

| Szenario | Lösung | Auflösung |
|---|---|---|
| HDMI vorhanden | direkt ans TV | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort | passiver DP→HDMI Adapter (~10€) | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort, 4K@60Hz gewünscht | aktiver DP 1.4→HDMI 2.0 Adapter (~25€) | 4K@60Hz |

**Für Jellyfin-Streaming (1080p Heimnetz) ist 1080p@60Hz der Sweet Spot** — passiver Adapter reicht völlig.

---

## Offene Fragen

- [ ] **Desktop-Umgebung**: GNOME, KDE Plasma oder XFCE? → ausprobieren
- [ ] **Remote-Zugriff**: Tailscale testen (bevorzugt) oder Cloudflare Tunnel?
- [ ] **Netzwerk**: Gigabit LAN zum Router vorhanden oder WLAN?
- [ ] **Nextcloud vs. Syncthing**: Nextcloud (vollwertige Cloud) oder Syncthing (simpler Sync)?
- [ ] **Blu-ray-Laufwerk**: Welches Modell? → MakeMKV-Kompatibilitätsliste prüfen vor dem Kauf
