# Homelab

Kompakter Heimserver auf Basis eines Dell OptiPlex 3090, der gleichzeitig mehrere Rollen übernimmt. Kein Enterprise-Setup, kein Gaming-PC — sondern ein sparsamer, dauerhaft laufender Knotenpunkt für persönliche Dienste, Emulation, Automatisierung und Heimstreaming.

**Status: Hardware gekauft, Linux-Installation ausstehend.**

---

## Hardware

**Dell OptiPlex 3090 Micro**
- CPU: Intel Core i5-10500T (6 Kerne / 12 Threads, 3,8 GHz Boost, **35W TDP**)
- iGPU: Intel UHD 630 — unterstützt Intel Quick Sync (H.264 + H.265/HEVC hardware-beschleunigt)
- RAM: 16 GB DDR4 (2 SODIMM-Slots, max. 64 GB — siehe RAM-Abschnitt)
- Storage: 256 GB SSD (intern, M.2 NVMe) + freier 2,5"-SATA-Schacht (7mm) für Erweiterung
- OS: Windows 11 (vorinstalliert) → wird ersetzt durch Linux

**Storage-Slots des 3090 Micro (laut Dell Service Manual):**
- **1× M.2-Slot** (2230/2280, PCIe NVMe) — belegt von der 256-GB-OS-SSD. Das ist der **einzige** nutzbare SSD-Slot.
- **1× M.2-Slot 2230** — **nur für die WLAN/BT-Karte**, nicht für SSDs nutzbar.
- **1× 2,5"-SATA-Schacht** (7mm, CMR-HDD oder SSD) — die einzige freie Erweiterungsmöglichkeit. Braucht einen Caddy/Rahmen (Dell-Teil 0HTR70), der bei manchen Konfigurationen fehlt (~5–15 € Drittanbieter).

> Eine zweite NVMe ist also **nicht** möglich (nur ein SSD-M.2-Slot, belegt vom OS). Für die Speicher-Strategie heißt das: Erweiterung läuft über USB (extern), nicht intern — siehe Abschnitt **Speicher**.

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

### Basis: Kubuntu 26.04 LTS ("Resolute Raccoon")

**Entschieden (Stand 01.07.2026):** Kubuntu 26.04 LTS statt 24.04 — bringt **KDE Plasma 6.6 nativ** mit (kein PPA-Umweg, der für 24.04 ohnehin nicht existiert), Support bis April 2031 (ESM sogar bis 2036 mit kostenlosem Ubuntu Pro, bis 5 Geräte). Da der OptiPlex ohnehin neu aufgesetzt wird (kein Upgrade von einem laufenden 24.04-System), ist der übliche Rat "auf den ersten Point-Release 26.04.1 warten" hier nicht relevant — der betrifft nur den `do-release-upgrade`-Pfad von bestehenden Installationen. Bei einer Neuinstallation reicht: 26.04 installieren, einmal vollständig updaten (`sudo apt update && sudo apt upgrade`) → identischer Patch-Stand wie mit 26.04.1-Medium.

**Zwei Breaking Changes gegenüber 24.04, für den Neuaufbau aber folgenlos:**
- **cgroup v1 wurde entfernt** (nur noch v2) — betrifft nur alte Container/Skripte mit hartcodierten v1-Annahmen, nicht eine frische Docker-Installation.
- **X11-Session wurde komplett entfernt** — nur noch Wayland (XWayland deckt alte Apps ab). Für Intel-iGPU (UHD 630) unkritisch; bekannte Startprobleme sind laut Recherche vor allem ein Nvidia-Thema. Jellyfin-VAAPI-Transcoding läuft ohnehin headless über `/dev/dri`, unabhängig vom Display-Server.

### Desktop-Umgebung: KDE Plasma (Kubuntu)

**Entschieden:** KDE Plasma statt GNOME/XFCE. Begründung:
- **KDE = maximal konfigurierbar** (jede Verhaltensweise direkt in den Systemeinstellungen einstellbar, kein Zwang zu Drittanbieter-Extensions) — passt zum Wunsch nach großen Einstellmöglichkeiten.
- **GNOME = bewusst simpel/opinionated** (viele Anpassungen nur über Extensions möglich) — nicht die Präferenz hier.
- Zusätzlicher Vorteil: KDE ist Basis von SteamOS/Steam Deck → gute Controller-Navigation für Emulation vom Sofa/TV aus, bleibt aber vollwertiger Desktop für Browser/YouTube/Nextcloud.
- **COSMIC Desktop (System76, Rust-basiert) geprüft und verworfen:** Distro-unabhängige DE, wird u.a. von CachyOS als Option angeboten (nicht umgekehrt — CachyOS "basiert" nicht auf COSMIC). Stand Mitte 2026 (v1.2.0) noch unausgereift für diesen Use Case: fraktionales Scaling fehlerhaft, Multi-Monitor teils defekt, vor allem **Remote-Display/Screen-Sharing (xdg-desktop-portal) noch nicht production-ready** — kritisch für geplanten Fernzugriff.
- **Bazzite/CachyOS/Nobara (Gaming-Distros) geprüft und verworfen:** primär auf reines Gaming/Handheld getrimmt, kein sauberer Docker-Weg (immutable Filesystem bei Bazzite → Podman/Quadlet statt Docker), CachyOS explizit als "Workstation, kein Server" positioniert — für einen unbeaufsichtigten 24/7-Homeserver unnötiges Risiko ohne Mehrwert.

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

### Upgrade auf 32 GB? — nur fürs lokale LLM

Der 3090 Micro hat **genau 2 SODIMM-Slots** (DDR4-2666/2933, max. 64 GB). **Ein 4-Riegel-Setup gibt es hier nicht** — 32 GB heißt immer **2× 16 GB**, nicht 4 Module (das wäre ein Tower/Desktop mit 4 DIMM-Slots).

- **Für den dokumentierten Stack reichen 16 GB locker** (~7–8 GB Worst Case). Kein Upgrade nötig.
- **32 GB lohnt nur**, wenn du `papagei-llm` lokal laufen lassen willst — dann braucht das Modell echten Puffer neben allem anderen.
- Kosten: ~40–60 € für 2× 16 GB (DDR4-Preise sind 2025/26 gestiegen — vorher prüfen).
- **Vorher checken**, ob die 16 GB als 1× 16 oder 2× 8 verbaut sind (`sudo dmidecode -t memory`):
  - 1× 16 → ein Slot frei, einfach 1× 16 dazustecken = 32 GB (Dual-Channel ✓).
  - 2× 8 → beide Slots voll, beide gegen 2× 16 tauschen.
- **Dual-Channel** (2 Riegel) ist relevant, weil die iGPU (UHD 630) sich den RAM teilt — bringt für Jellyfin-Transcoding & Emulation mehr Grafikbandbreite.

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

## Lokale Cloud: Nextcloud

**Entschieden (Stand 01.07.2026):** Nextcloud statt Syncthing — Syncthing kann strukturell keinen Browser-/App-Zugriff von unterwegs bieten (nur Administrations-Weboberfläche, kein Datei-Zugriff ohne eigenen Client auf jedem lesenden Gerät), reiner Peer-to-Peer-Sync zwischen eigenen Geräten. Da explizit Fernzugriff im Browser/per App gewünscht ist (nicht nur Gerätesync), fällt Syncthing als alleinige Lösung raus.

**Aktuell reiner Einzelnutzer-Betrieb geplant — Mehrbenutzerbetrieb aber als mögliche spätere Erweiterung im Hinterkopf behalten.** Nextcloud unterstützt das nativ (mehrere Accounts, Freigaben zwischen Nutzern, Gruppen/Berechtigungen) — bei der Ersteinrichtung nichts Spezielles zu beachten, das lässt sich jederzeit nachträglich aktivieren, ohne die Grundinstallation umzubauen.

**Open Source & Datenhoheit** (Kernmotivation: eigenständiges Homelab ohne Datenabfluss an Dritte):
- Lizenz **AGPLv3**, Code komplett öffentlich auf GitHub (`github.com/nextcloud/server` + separate App-Repos), entwickelt von Nextcloud GmbH (Deutschland)
- Kein Vergleich zu Google Drive/OneDrive/Dropbox: Serverlogik läuft vollständig lokal, Dateien verlassen den Server nie
- Zwei optionale externe Kontakte (keine Inhalte, nur Metadaten, beide abschaltbar): App-Store-Katalog-Abruf (`apps.nextcloud.com`) beim Installieren neuer Apps, sowie periodischer Update-Checker
- Wird u.a. von deutschen Bundesbehörden explizit wegen Datensouveränität gegenüber US-Cloud-Anbietern eingesetzt

**Bekannter Ressourcen-Vorbehalt:** Offiziell reichen 2 vCPU/4GB RAM für 1–5 Nutzer, es gibt aber dokumentierte Fälle (GitHub `nextcloud/all-in-one` Issues #6211 März 2025, #6962 Okt. 2025), in denen Preview-/Thumbnail-Generierung und Nextcloud-AIO-Bugs zu 25–34 GB RAM-Verbrauch statt der erwarteten 4–5 GB führten. Auf dem geteilten 16-GB-Optiplex neben Jellyfin-Transcoding und n8n ein reales Risiko bei Lastspitzen.

**Einrichtungs-Empfehlung:** Preview-Generierung von Anfang an deaktivieren/drosseln, RAM-Verbrauch in den ersten Wochen aktiv beobachten statt blind vertrauen.

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

Jellyfin läuft als Medienserver im Docker. Filme und Serien liegen auf der externen 8-TB-USB-HDD (zusammen mit den ROMs), Jellyfin indexiert alles automatisch mit Postern, Metadaten und Untertiteln. Streaming auf beliebige Geräte im Heimnetz — oder über Tailscale von unterwegs. Samba macht denselben Speicher als klassisches Netzlaufwerk zugänglich.

### Video-Format: H.265 (HEVC) — immer

H.265 spart ~50% Speicher gegenüber H.264 bei gleicher Qualität. Praktisch alle relevanten Abspielgeräte der letzten ~10 Jahre unterstützen es nativ.

**Zielauflösung: 1080p (Full HD) H.265 für alles.** Reicht für Handys, Tablets und selbst Mittelklasse-Smart-TVs locker — auf kleinen Displays ist der Unterschied zu 4K nicht sichtbar, und moderne TVs upscalen 1080p sauber. 4K nur als Ausnahme für einzelne Lieblingsfilme am großen TV (Direct Play, kein Upload nötig).

> **Native Auflösung, kein Upscaling.** Existiert eine Quelle nur in 720p, wird sie auch in **720p H.265** kodiert — Hochskalieren auf 1080p erfindet keine Details, macht die Datei nur größer. Jellyfin spielt gemischte Auflösungen problemlos.

**Dateigrößen (H.265):**
| Inhalt | Größe |
|---|---|
| Spielfilm 1080p (2h) | ~3–5 GB |
| Spielfilm 720p (2h, ältere Quellen) | ~1,5–2,5 GB |
| Serien-Episode 1080p (45 min) | ~0,7–1,5 GB |
| Serienstaffel 1080p (10 Folgen) | ~7–15 GB |
| Komplette Serie 1080p (5 Staffeln) | ~35–75 GB |
| Planungswert | ~4 GB/Film |
| **8 TB Festplatte (abzgl. ~1 TB ROMs)** | **~1.700+ Filme** |

> Eine 8-TB-Platte füllst du mit einer persönlichen Sammlung praktisch nie — bewusst großzügig dimensioniert, damit dauerhaft Ruhe ist. 4K würde die Dateien ~5–8× aufblähen und die Kapazität schnell fressen.

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

**Aufteilung nach Ersetzbarkeit** (statt nach Funktion — das ist der Schlüssel für die Backup-Frage):

| Speicher | Verwendung | Ersetzbar? |
|---|---|---|
| 256 GB M.2 SSD (intern, vorhanden) | Ubuntu + Docker-Volumes + alle Dienste + **Nextcloud-Daten (50–100 GB)** | OS ja, Nextcloud **nein** → Backup |
| **8 TB USB 3.0 HDD extern** | Filmbibliothek (Jellyfin) **+ ROMs** (alle Systeme) | ja (neu rippbar/dumpbar) |
| USB-Stick | Nextcloud-Backup + Passwörter, wichtige Dokumente | unersetzlich → offline |

> **Warum keine interne Zusatzplatte mehr?** Frühere Planung: interne 2,5"-HDD für ROMs + Nextcloud. Drei Gründe dagegen:
> 1. **Nur ein 2,5"-Schacht, gedeckelt bei 2 TB CMR** (7mm). Interne 2,5"-Platten sind außerdem zu einem Nischenprodukt geworden und absurd teuer (~94–150 € für 2 TB).
> 2. **Nextcloud passt auf die SSD.** Bei 50–100 GB (eigene Bilder/Dateien) liegt es zuverlässig auf der internen SSD — schnelle Sync, kein extra Bauteil.
> 3. **ROMs sind Bulk + ersetzbar** → gehören zu den Filmen auf die günstige externe 3,5"-Platte. Pro TB ist extern 2–3× billiger als intern.
>
> Ergebnis: **eine** externe 8-TB-Platte deckt Filme + ROMs ab, der 2,5"-Schacht bleibt leer (Reserve).

### ROMs auf HDD: kein Geschwindigkeitsproblem (SSD wäre Overkill)

Emulatoren laden Spiele beim Start komplett in den RAM. Die HDD-Geschwindigkeit betrifft nur die Ladezeit, nicht das Gameplay. Eine SSD bringt hier **keinen** spürbaren Vorteil — für ROMs ist eine HDD die richtige, günstige Wahl.

Zum Vergleich: die originale PS2-DVD las mit maximal ~5,4 MB/s — eine 3,5"-USB-HDD liest mit ~120–180 MB/s, also 20–30× schneller. Es gibt keine Emulations-Generation, bei der HDD-Geschwindigkeit ein Problem wäre. (Die ROMs liegen mit auf der externen 8-TB-Platte, USB-Autosuspend muss dafür aus sein — siehe unten.)

**Größenabschätzung ROMs (persönliche Sammlung):**

| System | Größe pro Spiel | 50 Spiele |
|---|---|---|
| NES / SNES / GBA | < 10 MB | < 1 GB |
| N64 | 8–64 MB | ~2 GB |
| Saturn / Dreamcast | 700 MB – 1 GB | ~50 GB |
| PS1 | 600–700 MB | ~35 GB |
| PS2 | 2–8 GB | ~200–400 GB |

Etwa **1 TB** reicht für eine persönliche Sammlung über alle Systeme — auf der 8-TB-Platte ist das problemlos mit drin, der Rest gehört den Filmen.

### Die externe Platte: Zwei Optionen (Stand Juli 2026)

**Ursprüngliche Wahl (WD Elements Desktop 8 TB) nach tieferer Recherche revidiert.** Die WD Elements verbaut intern eine **WD80EDAZ** (WD Blue Consumer-Klasse) — nicht die früher angenommene HGST Ultrastar. Das bedeutet: nur 2.400 h/Jahr Workload-Limit (= 6,5 h/Tag), nicht für 24/7 spezifiziert, Temperaturen unter Last bis **60 °C**. Für einen Server der nie ausgeht ein echtes Langzeit-Risiko. Seagate Expansion Desktop 8 TB ebenfalls ausgeschlossen — verbaut SMR-Platte (Barracuda Compute).

**Zwei sinnvolle Optionen:**

| | Option A — Gebraucht | Option B — Neu |
|---|---|---|
| **Komponenten** | Seagate Exos 7E8 8TB (ST8000NM0055) + Inateck FE3002 | Seagate IronWolf 8TB (ST8000VN004) + Inateck FE3002 |
| **Preis** | ~107–125 € | ~342 € |
| **24/7-spezifiziert** | ✅ (550 TB/Jahr Workload) | ✅ (180 TB/Jahr Workload) |
| **CMR** | ✅ | ✅ |
| **Temperatur** | ~42 °C (Heliumfüllung) | ~45 °C |
| **Garantie** | keine (Privatverkauf) | 3 Jahre (Hersteller) |
| **Risiko** | SMART-Werte prüfen vor Kauf | keins |

**Option A — Gebraucht:** Exos aus Rechenzentrums-Ausmusterung, auf Kleinanzeigen/eBay (~80–100 €) + Inateck FE3002 Gehäuse (~25 €). Enterprise-Klasse, läuft kühler als Consumer-Platten. Vor Kauf SMART-Daten vom Verkäufer anfordern (`smartctl -a /dev/sdX` oder CrystalDiskInfo-Screenshot):

| SMART-Attribut | Muss sein |
|---|---|
| `Reallocated_Sector_Ct` | **0** — sonst nicht kaufen |
| `Current_Pending_Sector` | **0** |
| `Offline_Uncorrectable` | **0** |
| `Power_On_Hours` | **unter 40.000 h** |

Modellnummer auf SATA-Variante prüfen: `ST8000NM0055` (SATA ✅) — nicht `ST8000NM0016` (SAS ❌, funktioniert nicht an normalem USB-Gehäuse).

**Option B — Neu:** IronWolf ist eine echte NAS-Platte, 3 Jahre Garantie, keine Fragezeichen. Inateck FE3002 spezifisch kaufen (ASM1153E-Chip, UASP, eigenes 12V/2A-Netzteil, bis 20 TB).

> **Warum 8 TB statt 4 TB?** 4-TB-Externe sind ausnahmslos **SMR** und teils sogar teurer als die 8-TB-CMR-Optionen. Direkt 8 TB — bessere Technik, doppelter Platz.

> **Warum SMR hier doof wäre:** SMR schreibt Spuren überlappend (wie Dachschindeln). Beim **Schreiben großer Mengen am Stück** bricht die Rate ein, sobald der Cache voll ist. **Parallel lesen + schreiben** (Jellyfin streamt, während archiviert wird) lässt SMR stottern. CMR bleibt durchgehend schnell.

> **Finger weg von WD My Book / Easystore:** Feste AES-Hardware-Verschlüsselung im USB-Chip — stirbt der Controller, sind die Daten unwiederbringlich verloren. WD Elements hat diesen Chip nicht, die hier empfohlenen Optionen ebenfalls nicht.

### Filmbibliothek: kein Geschwindigkeitsproblem

Die USB 3.0-Verbindung ist nicht der Flaschenhals — die HDD selbst ist es, und sie ist trotzdem mehr als schnell genug:

| | Geschwindigkeit |
|---|---|
| 3,5" HDD Lesegeschwindigkeit | ~120–180 MB/s |
| 1080p H.265 Stream | ~5–15 MB/s |
| 3 gleichzeitige Streams | ~45 MB/s → HDD schafft das 3–4× über |

### Linux-Setup der externen Platte (24/7-Pflicht)

1. **Auf ext4 formatieren** (nicht das Werks-NTFS/exFAT) — Journaling schützt bei Stromausfall, kein 4-GB-Dateilimit, saubere Linux-Rechte für Jellyfin/Samba. Samba teilt ext4 problemlos.
2. **Per UUID in `/etc/fstab` mit Option `nofail`** einbinden — so bootet der Server auch, wenn die Platte mal nicht dranhängt.
3. **USB-Autosuspend hart deaktivieren** — in `/etc/default/grub` den Kernel-Parameter `usbcore.autosuspend=-1` ergänzen, dann `sudo update-grub` + Neustart. Sonst drohen I/O-Fehler / spontanes Unmount im Dauerbetrieb.
4. **HDD-Parken deaktivieren** (gegen unnötige Start/Stop-Zyklen):
   ```bash
   hdparm -S 0 /dev/sdX
   ```
5. **Kabel/Port:** Standard-USB-3.0-Micro-B-Kabel liegt bei, kommt in einen USB-A-Port am OptiPlex (kein Spezialkabel, kein ungepowerter Hub). HDD-Tempo liegt weit unter USB-3.0-Bandbreite → kein Flaschenhals.

### Backup-Strategie

Backup nur für das **Unersetzliche** — Filme und ROMs sind ersetzbar (neu rippbar/dumpbar), nur die Nextcloud-Daten sind es nicht:

| Was | Backup wie |
|---|---|
| Filmbibliothek + ROMs (8 TB extern) | kein separates Backup — neu rippbar/dumpbar |
| **Nextcloud-Daten (auf SSD, 50–100 GB)** | USB-Stick (oder kleine externe SSD) als Kopie |
| Dokumente / Passwörter | USB-Stick, räumlich getrennt aufbewahren |

Da Nextcloud nur 50–100 GB groß ist, reicht ein **128–256-GB-USB-Stick** als Backup-Ziel locker — die teure zweite Platte braucht es nicht. Wer doch ein vollständiges, automatisches Backup will: zweite externe HDD + `rsync` per systemd-Timer reicht vollkommen — keine Cloud nötig.

---

## Netzwerk & Remote-Zugriff

- **LAN:** Gigabit-Kabel bevorzugt — nativer LAN-Port defekt, USB 3.0 Gigabit Adapter (~15€) als Dauerlösung. Realer Durchsatz ~700–900 Mbps, für Streaming und alle Dienste völlig ausreichend.
- **SSH:** direkt im lokalen Netz, kein Extra-Setup nötig

### Remote-Zugriff: Tailscale (empfohlen)

Tailscale erstellt ein privates WireGuard-Mesh-Netzwerk zwischen eigenen Geräten. Kein Port-Forwarding, funktioniert hinter CGNAT, keine feste IP nötig.

- Installation auf OptiPlex + alle Client-Geräte (Handy, Laptop)
- Jedes Gerät bekommt eine stabile interne IP (`100.x.x.x`)
- Jellyfin, Nextcloud etc. erreichbar als wären sie im Heimnetz
- Kostenlos: 6 Nutzer/Tailnet, unbegrenzte Geräte (Stand Juli 2026)

> Tailscale erfordert die App auf jedem Client-Gerät. Wer Dienste ohne App für andere teilen möchte → Cloudflare Tunnel als Alternative.

### Entschieden: Hybrid-Setup — Tailscale + Cloudflare Tunnel

**Recherchiert und entschieden (Stand 01.07.2026):** Kein Entweder-Oder, beide laufen konfliktfrei parallel (keine Portkonflikte).

| Kriterium | Tailscale | Cloudflare Tunnel + Access |
|---|---|---|
| Free-Tier | 6 Nutzer/Tailnet, unbegrenzte Geräte | Bis 50 Nutzer, 100MB Body-Size-Limit |
| Zugriff für Gelegenheitsgäste | Funnel: kein Client nötig, aber noch **Beta**, undokumentierte Bandbreitenlimits | Access mit E-Mail-OTP: 6-stelliger Code per Mail, kein Client nötig |
| Granularer Gastzugang | ACL-Regeln (`dst: host:port`) + Tailscale SSH | "Access for Infrastructure" mit Service Tokens (seit Feb. 2026) |
| Setup-Aufwand | Gering, kein DNS/Domain nötig | Höher — eigene Domain + Zero-Trust-Konfiguration |
| Performance | Meist direktes WireGuard-P2P, bei CGNAT via Relay etwas langsamer | 20–50ms Zusatzlatenz, für Streaming unkritisch |
| Ausfälle 2025/26 | Kleinere Control-Plane-Vorfälle, bestehende Verbindungen liefen meist weiter | Mehrere größere Outages (u.a. 18.11.2025 ~6h, 20.02.2026 ~6h) |
| Datenschutz | Ende-zu-Ende WireGuard, kein Klartext-Zugriff durch Dritte | TLS wird an Cloudflare-Edge terminiert — Cloudflare sieht technisch den Klartext |

**Aufteilung:**
- **Tailscale** für den eigenen dauerhaften Zugriff (Nextcloud, Home Assistant, n8n, SSH) und für den **Kollegen-Zugang zum Trading-Bot** — eigene ACL-Regel (`dst: bot-host:port`) + zusätzlich `ForceCommand`/rbash im Container selbst als Defense-in-Depth. Kein öffentlicher Angriffspunkt, Ende-zu-Ende verschlüsselt.
- **Cloudflare Tunnel + Access (E-Mail-OTP)** nur für **Jellyfin-Freigabe an Freunde/Familie**, die keinen eigenen Tailscale-Client installieren sollen. Tailscale Funnel wäre die naheliegende Alternative, ist aber noch Beta mit undokumentierten Limits — Cloudflares E-Mail-OTP-Zugang ist ausgereifter und pro E-Mail-Adresse granular steuerbar.

> **Rechtlicher Hinweis:** Cloudflares ToS (Abschnitt 2.8) behandelt Mediendateien-Streaming in einer Grauzone — laut Recherche aktuell bei privater, nicht-kommerzieller Nutzung nicht durchgesetzt, aber erwähnenswert.

### Streaming von außerhalb — Upload-Bedarf

**Vorhandener Upload zu Hause: 50 Mbit/s** — komfortabel überdurchschnittlich.

| Qualität | Upload pro Stream | Bei 50 Mbit/s |
|---|---|---|
| 1080p H.265 | ~5–15 Mbit/s | ✅ **3–4 gleichzeitige** Streams von außen |
| 1080p H.264 | ~8–25 Mbit/s | ✅ 2–3 gleichzeitig |
| 4K H.265 | ~25–40 Mbit/s | ⚠️ **ein** Stream machbar, aber selten nötig |

Heißt: Für 1080p (das Zielformat) reicht der Upload locker — auch wenn mehrere Personen gleichzeitig von außen schauen. 4K von unterwegs lohnt nicht (am Handy/Tablet nicht sichtbar, große Dateien, Transcoding-Last).

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
| **8 TB extern** — Option A: Seagate Exos 7E8 8TB `ST8000NM0055` (gebraucht, SMART prüfen) + Inateck FE3002 Gehäuse · Option B: Seagate IronWolf 8TB `ST8000VN004` (neu) + Inateck FE3002 | Filme (Jellyfin) **+ ROMs** | ~107–125 € (A) / ~342 € (B) |
| **Externes USB Blu-ray-Laufwerk** | Discs rippen mit MakeMKV | ~60–80 € |
| USB-Stick (128–256 GB) | Nextcloud-Backup + wichtige Dateien | ~15–25 € |
| **Gamepad** — 8BitDo Pro 2 (SNES-Layout, 2D-Fokus) oder Ultimate 2 (Xbox-Layout, Analog-Fokus) | Emulation | ~45–60 € |

> **Bewusst nicht gekauft:**
> - *Interne 2,5"-HDD* — überteuert (~94–150 € für 2 TB) und überflüssig, da Nextcloud auf die SSD passt und ROMs auf die externe Platte gehen.
> - *4-TB-Externe* — lohnt nicht: ausnahmslos SMR, teils teurer als die 8-TB-CMR. Direkt 8 TB nehmen.
>
> **Optional (nur bei Bedarf):**
> - *2× 16 GB DDR4 SODIMM* (~40–60 €) — nur falls `papagei-llm` lokal laufen soll. Für den Standard-Stack reichen die 16 GB. Kein 4-Riegel-Setup möglich (nur 2 Slots).
> - *2,5"-Caddy 0HTR70* (~5–15 €) — nur falls der Schacht später doch genutzt wird und der Rahmen fehlt.

### Display-Anschluss

Die Artikelliste zeigt **DisplayPort + VGA** — kein HDMI. Standard-Spezifikation des OptiPlex 3090 Micro hat normalerweise HDMI 1.4 + DisplayPort 1.4. Erst beim Auspacken prüfen.

| Szenario | Lösung | Auflösung |
|---|---|---|
| HDMI vorhanden | direkt ans TV | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort | passiver DP→HDMI Adapter (~10€) | 4K@30Hz / 1080p@60Hz |
| Nur DisplayPort, 4K@60Hz gewünscht | aktiver DP 1.4→HDMI 2.0 Adapter (~25€) | 4K@60Hz |

**Für Jellyfin-Streaming (1080p Heimnetz) ist 1080p@60Hz der Sweet Spot** — passiver Adapter reicht völlig.

---

## Gastzugang für Kollegen (Trading Bot)

Sicherer Zugang für eine Vertrauensperson die nur am Trading Bot arbeiten soll — kein Zugang zu anderen Diensten, kein sudo auf das System.

### Voraussetzung: Tailscale

Kollege installiert Tailscale, du lädst ihn per E-Mail ins Tailnet ein. Er erreicht den OptiPlex dann über die Tailscale-IP — keine offenen Ports, kein VPN-Gebastel.

### Einrichtung

```bash
# 1. Dedizierter User anlegen
sudo useradd -m -s /bin/bash kollege
sudo passwd kollege

# 2. SSH-Key des Kollegen hinterlegen
sudo mkdir -p /home/kollege/.ssh
sudo nano /home/kollege/.ssh/authorized_keys
# → Public Key des Kollegen einfügen
sudo chown -R kollege:kollege /home/kollege/.ssh
sudo chmod 700 /home/kollege/.ssh

# 3. Sudoers-Regel — nur Bot-spezifische Docker-Befehle erlaubt
sudo nano /etc/sudoers.d/kollege
```

```
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker restart trading-bot
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker logs trading-bot
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker compose -f /home/kollege/trading-bot/docker-compose.yml up -d
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker compose -f /home/kollege/trading-bot/docker-compose.yml down
```

### Was der Kollege dann tun kann

```bash
# Neue Bot-Version hochladen
scp bot.py kollege@100.x.x.x:~/trading-bot/

# Einloggen und Bot neustarten
ssh kollege@100.x.x.x
sudo docker restart trading-bot
sudo docker logs trading-bot
```

Er sieht nur sein Home-Verzeichnis und kann nur den Trading-Bot-Container steuern. Alle anderen Container (Jellyfin, Nextcloud, AdGuard etc.) und das restliche Dateisystem sind für ihn nicht erreichbar.

---

## Offene Fragen

- [x] **Desktop-Umgebung**: KDE Plasma (Kubuntu 26.04 LTS) — siehe Abschnitt "Betriebssystem: Linux"
- [x] **Remote-Zugriff**: Hybrid — Tailscale (eigener Zugriff + Kollege) + Cloudflare Tunnel (Jellyfin-Freigabe an Freunde/Familie), siehe Abschnitt "Netzwerk & Remote-Zugriff"
- [ ] **Netzwerk**: Gigabit LAN zum Router vorhanden oder WLAN?
- [x] **Nextcloud vs. Syncthing**: Nextcloud — siehe Abschnitt "Lokale Cloud: Nextcloud" (aktuell Einzelnutzer, Mehrbenutzerbetrieb später möglich)
- [ ] **Blu-ray-Laufwerk**: Welches Modell? → MakeMKV-Kompatibilitätsliste prüfen vor dem Kauf
