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

### Die externe Platte: WD Elements Desktop 8 TB

**Empfehlung: WD Elements Desktop 8 TB (WDBWLG0080HBK), ~208 €.** Recherchiert und bewusst gewählt:

| Kriterium | WD Elements 8 TB | Warum wichtig |
|---|---|---|
| **CMR** (kein SMR) | ✅ HGST Ultrastar He8 intern | s. SMR-Hinweis unten |
| **Keine Hardware-Verschlüsselung** | ✅ (USB-3.0-Version) | bei Gehäuse-Defekt Platte einfach woanders auslesbar |
| Eigenes Netzteil (3,5" Desktop) | ✅ | keine Last/Disconnect wie bei bus-powered 2,5" |
| Kapazitätsgrenze über USB | keine | 3,5" hat die 2-TB-CMR-Grenze der 2,5"-Platten nicht |

> **Warum 8 TB statt 4 TB?** 4-TB-Externe sind ausnahmslos **SMR** und teils sogar **teurer** (WD 4 TB ~252 €!) als die 8-TB-CMR. Der einzige günstige 4-TB-Tipp (Seagate ~120–145 €) ist SMR und spart nur ~60–80 € bei halber Kapazität. Also direkt zur 8-TB-CMR — bessere Technik, doppelter Platz, kaum Aufpreis.

> **Warum SMR hier doof wäre:** SMR schreibt Spuren überlappend (wie Dachschindeln). Lesen ist gleich schnell, aber beim **Schreiben großer Mengen am Stück** (Filmsammlung/ROMs draufkopieren) bricht die Rate ein, sobald der Cache voll ist (~100–200 GB → dann 30–60 statt 150–200 MB/s). Und **parallel lesen + schreiben** (Jellyfin streamt, während archiviert wird) lässt SMR stottern. CMR bleibt durchgehend schnell. Da 8 TB ohnehin nur als CMR sinnvoll ist, umgeht man das Problem gratis.

> **Finger weg von WD My Book / Easystore:** Die haben eine AES-Hardware-Verschlüsselung fest im USB-Chip — stirbt der Controller, sind die Daten **unwiederbringlich** verloren, selbst für Profi-Datenrettung. WD Elements hat diesen Chip nicht.

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
- Kostenlos bis 100 Geräte / 3 Nutzer

> Tailscale erfordert die App auf jedem Client-Gerät. Wer Dienste ohne App für andere teilen möchte → Cloudflare Tunnel als Alternative.

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
| **8 TB USB 3.0 HDD extern** — WD Elements Desktop (WDBWLG0080HBK), CMR, ohne Encryption-Bridge | Filme (Jellyfin) **+ ROMs** | ~208 € |
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

- [ ] **Desktop-Umgebung**: GNOME, KDE Plasma oder XFCE? → ausprobieren
- [ ] **Remote-Zugriff**: Tailscale testen (bevorzugt) oder Cloudflare Tunnel?
- [ ] **Netzwerk**: Gigabit LAN zum Router vorhanden oder WLAN?
- [ ] **Nextcloud vs. Syncthing**: Nextcloud (vollwertige Cloud) oder Syncthing (simpler Sync)?
- [ ] **Blu-ray-Laufwerk**: Welches Modell? → MakeMKV-Kompatibilitätsliste prüfen vor dem Kauf
