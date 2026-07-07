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
- `systemd` für automatisches Starten aller Dienste (Nextcloud, Jellyfin, AdGuard etc.) — falls der Trading Bot später doch aktiviert wird, zusätzlich relevant für dessen Timer
- Kein Windows Update das die Dienste unterbricht
- SSH out-of-the-box, weniger RAM-Overhead für das OS selbst
- Alle Server-Dokumentationen (Nextcloud, Home Assistant, AdGuard etc.) primär auf Linux geschrieben

### Basis: Kubuntu 26.04 LTS ("Resolute Raccoon")

**Entschieden (Stand 01.07.2026):** Kubuntu 26.04 LTS statt 24.04 — bringt **KDE Plasma 6.6 nativ** mit (kein PPA-Umweg, der für 24.04 ohnehin nicht existiert), Support bis April 2031 (ESM sogar bis 2036 mit kostenlosem Ubuntu Pro, bis 5 Geräte). Da der OptiPlex ohnehin neu aufgesetzt wird (kein Upgrade von einem laufenden 24.04-System), ist der übliche Rat "auf den ersten Point-Release 26.04.1 warten" hier nicht relevant — der betrifft nur den `do-release-upgrade`-Pfad von bestehenden Installationen. Bei einer Neuinstallation reicht: 26.04 installieren, einmal vollständig updaten (`sudo apt update && sudo apt upgrade`) → identischer Patch-Stand wie mit 26.04.1-Medium.

**Zwei Breaking Changes gegenüber 24.04, für den Neuaufbau aber folgenlos:**
- **cgroup v1 wurde entfernt** (nur noch v2) — betrifft nur alte Container/Skripte mit hartcodierten v1-Annahmen, nicht eine frische Docker-Installation.
- **X11-Session wurde komplett entfernt** — betrifft primär GNOME; bei KDE/Kubuntu könnte X11 je nach Version noch wählbar sein, gilt aber ebenfalls nicht mehr als offiziell unterstützt — vor der Installation kurz prüfen. Für den hier ohnehin geplanten Wayland/VAAPI-Betrieb irrelevant: Für Intel-iGPU (UHD 630) unkritisch, bekannte Startprobleme sind laut Recherche vor allem ein Nvidia-Thema. Jellyfin-VAAPI-Transcoding läuft ohnehin headless über `/dev/dri`, unabhängig vom Display-Server.

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
| Docker (alle Dienste inkl. Jellyfin) | ~2,5–3,5 GB |
| Trading Bot + FinBERT (nur falls aktiviert — siehe Status: optional/zurückgestellt) | ~2,0 GB |
| PCSX2 aktiv | ~1,5 GB |
| IBKR TWS (nur falls Trading Bot aktiviert) | ~800 MB |
| OS-Kernel + Puffer | ~500 MB |
| **Gesamt** | **~8,5–10 GB** |

16 GB bieten selbst im Worst Case noch ~35–45% freien Headroom für den dokumentierten Stack. Kein RAM-Upgrade nötig.

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
| Monitoring / Alerting | Uptime Kuma | geplant |
| Trading Bot | Python-Bot (siehe `ideas/trading-bot`) | optional / zurückgestellt |

**Entschieden: Home Assistant Container (Docker-Image), nicht Supervised/HAOS.** Passt zum Prinzip "eine `docker-compose.yml` pro Dienst" — Supervised würde den ganzen Host übernehmen wollen und mit dem restlichen Docker-Stack kollidieren. Der Add-on-Store entfällt dadurch, die meisten gängigen Integrationen (inkl. MQTT) funktionieren trotzdem.

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

**Entschieden: Plain Nextcloud statt AIO.** Die RAM-Ausreißer betreffen spezifisch Nextcloud AIO (All-in-One) — das Komplettpaket mit eigenen Containern für Collabora (Browser-Office-Editor), Talk (Video-Calls inkl. TURN-Server), Elasticsearch-Volltextsuche und ClamAV-Virenscan. Jeder dieser Zusatzdienste bringt seinen eigenen RAM-Hunger mit (Elasticsearch allein oft 1–2 GB Java-Heap). Da weder Office-Kollaboration im Browser noch Video-Calls noch Volltext-Dateiinhaltssuche gebraucht werden, entfällt der gesamte Grund für AIO.

Stattdessen: **Plain-Docker-Setup** mit dem offiziellen `nextcloud:apache`-Image + eigener MariaDB + Redis (Caching) — passend zum Prinzip "eine `docker-compose.yml` pro Dienst". Typischer RAM-Verbrauch für Einzelnutzer: ~300–600 MB statt der AIO-Risiko-Range. Fehlende Features (Collabora, Talk, Volltextsuche) lassen sich bei Bedarf jederzeit als einzelne zusätzliche Container nachrüsten, ohne die Basis-Installation umzubauen.

**Einrichtungs-Empfehlung:** Preview-Generierung im Blick behalten, RAM-Verbrauch in den ersten Wochen aktiv beobachten statt blind vertrauen.

**Cron statt AJAX:** In den Nextcloud-Admin-Einstellungen (Grundeinstellungen → Hintergrundaufgaben) von "AJAX" auf "Cron" umstellen — sonst laufen Vorschaubilder, Datei-Scans und Benachrichtigungen nur unzuverlässig (nur wenn gerade ein Browser-Tab offen ist). Host-seitig per `systemd`-Timer alle 5 Minuten:

```
docker exec -u www-data nextcloud php occ system:cron
```

---

## Software-Stack

```
┌─────────────────────────────────────┐
│            OptiPlex 3090            │
│       Kubuntu 26.04 LTS             │
│                                     │
│  Docker-Netzwerk: homelab_net       │
│  ├── AdGuard Home      (DNS, :53)   │
│  ├── Caddy             (:443)       │
│  ├── Nextcloud         (intern :80) │
│  ├── Jellyfin          (:8096)      │
│  ├── Samba             (:445)       │
│  ├── Home Assistant    (:8123)      │
│  ├── Mosquitto MQTT    (:1883)      │
│  ├── n8n               (:5678)      │
│  └── Uptime Kuma       (:3001)      │
│                                     │
│  Nativ (kein Container):            │
│  ├── PCSX2 / RetroArch (Emulation) │
│  ├── IBKR TWS          (Broker)    │
│  └── Trading Bot       (Python)    │
└─────────────────────────────────────┘
```

Alle Server-Dienste in Docker → einfaches Update, saubere Isolation, einheitliche `docker-compose.yml` pro Dienst.

**Gemeinsames Docker-Netzwerk `homelab_net`:** Vor allen Diensten einmalig anlegen:

```bash
docker network create homelab_net
```

Jede `docker-compose.yml` bindet sich per `networks: {default: {name: homelab_net, external: true}}` daran an, statt ein eigenes isoliertes Netzwerk pro Compose-Datei aufzumachen. Grund: Container müssen sich fast durchweg gegenseitig erreichen (Home Assistant ↔ Mosquitto per MQTT, Uptime Kuma überwacht alle anderen Container per Name statt IP) — ein einziges gemeinsames Netzwerk ist hier einfacher als feingranulare Segmentierung pro Dienst-Paar.

---

## NAS & Heimstreaming

### Konzept

Jellyfin läuft als Medienserver im Docker. Filme und Serien liegen auf der externen 8-TB-USB-HDD (zusammen mit den ROMs), Jellyfin indexiert alles automatisch mit Postern, Metadaten und Untertiteln. Streaming auf beliebige Geräte im Heimnetz — oder über Tailscale von unterwegs. Samba macht denselben Speicher als klassisches Netzlaufwerk zugänglich.

> **Samba Auto-Discovery:** Für Auto-Discovery im Explorer/Finder (Freigabe taucht automatisch im Netzwerk auf) sind zusätzlich UDP 137/138 und TCP 139 nötig — oder einfacher: `network_mode: host` wie bei AdGuard. Ohne das ist nur die manuelle UNC-Pfad-Eingabe (`\\server\freigabe`) möglich.

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
2. In `docker-compose.yml` das DRI-Gerät durchreichen und die `render`-Gruppe mitgeben:
   ```yaml
   devices:
     - /dev/dri:/dev/dri
   group_add:
     - "<render-gid>"
   ```
   GID vorher auf dem Host ermitteln: `getent group render`
3. In Jellyfin: Dashboard → Wiedergabe → Transkodierung → **Intel QuickSync** aktivieren

> Ohne Schritt 2 sieht der Container die iGPU nicht — kein Hardware-Transcoding, alles läuft dann auf der CPU. Auch mit sichtbarem `/dev/dri`-Device, aber ohne das `group_add`-GID-Mapping, kann trotzdem ein Permission-Denied auftreten.

### Medien-Quellen: Blu-rays rippen

Blu-rays und DVDs kaufen und rippen:
- **MakeMKV** — ripped die Disc zur MKV-Datei (verlustfrei, große Datei). Rohdatei temporär auf die externe 8-TB-HDD schreiben (genug Platz), nicht auf die knappe interne 256-GB-SSD.
- **Handbrake** — komprimiert auf H.265 1080p (empfohlen für Speichereffizienz). Nach der Kompression die MakeMKV-Rohdatei löschen.
- Externes USB Blu-ray-Laufwerk nötig (~60–80€) — auf MakeMKV-Kompatibilitätsliste prüfen
- Für **4K UHD Blu-rays**: nur bestimmte Laufwerke (ggf. Firmware-Modifikation nötig), deutlich aufwändiger

> **Rechtlicher Hinweis (Deutschland):** § 95a UrhG verbietet das Umgehen technischer Schutzmaßnahmen (AACS-Verschlüsselung auf Blu-rays) — auch für Privatkopien. Das Rippen kommerzieller Blu-rays ist nach aktuellem deutschen Recht rechtlich problematisch, auch wenn es in der Praxis nicht verfolgt wird.

---

## Monitoring & Alerting

**Geplant: Uptime Kuma** — leichtgewichtiges Self-Hosted-Dashboard (Node.js + SQLite, ein Container) für Uptime-Monitoring. UI-basiert konfigurierbar (kein Plugin-System): beliebig viele Monitore (HTTP-Check, TCP-Port, Ping, DNS, Docker-Container-Health, oder ein Push-Heartbeat für eigene Skripte/Bots), pro Monitor eigene Benachrichtigungs-Integration (ntfy, Telegram, Discord, E-Mail, Webhook, ~100 Optionen).

Sinn: merken, wenn ein Dienst (Nextcloud, Jellyfin, AdGuard etc.) abstürzt oder ein Container hängt — statt es zufällig zu bemerken.

> Alternative, falls später Config-als-Code statt Klick-UI gewünscht ist: **Gatus** (YAML-konfiguriert, GitOps-freundlich) oder **Healthchecks.io** (self-hostbar, spezialisiert auf "hat sich mein Cronjob/Bot heute gemeldet?").

**SMART-Monitoring laufend statt nur beim Kauf:** Uptime Kuma prüft Dienst-Verfügbarkeit, aber keine Platten-Gesundheit über die Zeit. Ergänzend **Scrutiny** (eigener Container, SMART-Historie + Trend-Warnung fürs Web-Dashboard) für die externe HDD und die interne SSD einrichten — alternativ minimal ein `smartd`-Daemon mit Mail-Alert.

Noch nicht ausgearbeitet — Details (welche Dienste, welcher Notification-Kanal) folgen bei Bedarf.

---

## Optional / Zurückgestellt

Ideen, die interessant klingen, aber aktuell nicht verfolgt werden:

- **Vaultwarden** (self-hosted Bitwarden-kompatibler Passwortmanager) — nur relevant, falls doch mehr als Hauptpasswort + Varianten verwaltet werden soll. Geparkt.
- **Immich** (self-hosted Foto-Verwaltung mit KI-Gesichts-/Objekterkennung) — RAM-hungrigster Kandidat der Liste (Server + Postgres + Redis + ML-Container, ~1,5–2,5 GB idle, spürbar mehr bei aktiver Gesichtserkennung/großem Scan). Geparkt, ggf. später bei Bedarf nachziehen.
- **USV (UPS)** — erst relevant, sobald der Server 24/7 läuft. Aktuell kein Dauerbetrieb geplant, daher zurückgestellt.

---

## Einrichtungsreihenfolge (Setup-Plan)

1. **Docker-Netzwerk anlegen** — `docker network create homelab_net`, bevor irgendein Dienst startet
2. **Linux installieren** — Ubuntu-Variante wählen, Windows ersetzen
3. **Grundkonfiguration** — SSH aktivieren, Updates, Benutzer einrichten
4. **Docker installieren** — offizielle Ubuntu-Anleitung
5. **`intel-media-va-driver` installieren** — für Quick Sync / VAAPI
6. **AdGuard Home** — zuerst, da DNS-Blocker für alle anderen Geräte im Netz (im Router zusätzlich einen Fallback-DNS, z. B. `1.1.1.1`, als Sekundär-Server eintragen — sonst ist bei Server-Neustart/-Wartung kurzzeitig das ganze Heimnetz ohne DNS)
   > **Vor dem ersten Start:** Ubuntu/Kubuntu bringt `systemd-resolved` mit, das per Default einen DNS-Stub-Listener auf Port 53 belegt — kollidiert mit AdGuard Home, das denselben Port braucht. Ohne Fix schlägt der Container-Start mit "port already in use" fehl.
   > **Fix:** in `/etc/systemd/resolved.conf` die Zeile `DNSStubListener=no` setzen, dann `sudo systemctl restart systemd-resolved` — erst danach den AdGuard-Container starten.
   > **Zusätzlich empfohlen:** `network_mode: host` statt reinem Port-Mapping in der `docker-compose.yml`, sonst sieht AdGuard im Log nur die Docker-Gateway-IP statt der echten Client-IPs aus dem LAN.
   > **Docker-DNS-Bypass vermeiden:** Ohne weitere Konfiguration nutzen andere Container Dockers eigenen internen DNS-Resolver und umgehen AdGuard damit faktisch. In `/etc/docker/daemon.json`: `{"dns": ["<AdGuard-Host-IP>"]}` setzen, danach Docker-Daemon neu starten (`sudo systemctl restart docker`). Alternativ pro Compose-Datei `dns: [<AdGuard-IP>]` setzen.
   > **Firewall für `network_mode: host`:** Da dieser Modus die normale Docker-Netzwerkisolation aufhebt, `ufw` oder `nftables` einrichten und nur die benötigten Ports (53 für DNS, AdGuard-Web-UI-Port) fürs LAN-Subnetz freigeben, alles andere blocken.
7. **Jellyfin** — Medienserver, VAAPI konfigurieren, externe HDD einbinden
8. **Samba** — Netzlaufwerk einrichten
9. **Nextcloud** — lokale Cloud einrichten, Speicher einbinden, Cron statt AJAX
10. **Mosquitto MQTT** — Broker für ESP32-Projekte (Companion-Watch, Auto-Bewässerung), zuerst, da Home Assistant die MQTT-Integration gegen den bereits laufenden Broker konfiguriert
11. **Home Assistant** — Heimautomatisierung, MQTT-Integration gegen Mosquitto
12. **n8n** — Workflow-Automatisierung
13. **Uptime Kuma** — Monitore für alle bisherigen Dienste anlegen
14. **Tailscale** — Remote-Zugriff einrichten, danach HTTPS via Caddy für Nextcloud
15. **Cloudflare Tunnel + Access** — nur für Jellyfin-Freigabe an Freunde/Familie
16. **Backup einrichten** — `rsync`-Hardlink-Versionierung auf USB-Stick, Restore-Test
17. **Emulation** — PCSX2 + RetroArch nativ installieren

> **Trading Bot** (IBKR TWS + Python-Umgebung + systemd-Timer) ist **nicht** Teil dieser Reihenfolge — Status optional/zurückgestellt, siehe Abschnitt "Trading Bot".

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

Beide Varianten kabellos (2.4GHz-Dongle + Bluetooth), Plug-and-Play unter Kubuntu 26.04 (aktueller Kernel, kein Extra-Treiber), SDL-kompatibel → direkt in RetroArch & PCSX2 erkannt.

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

## Trading Bot (optional / zurückgestellt)

> Status offen — evtl. nicht weiterverfolgt. Details unten bleiben als Referenz stehen, falls die Idee doch aufgegriffen wird.

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

**Auth statt offener Broker:** `allow_anonymous false` in `mosquitto.conf`, Benutzer/Passwort mit `mosquitto_passwd` anlegen. `companion-watch` und `auto-bewaesserung` entsprechend mit den vergebenen Zugangsdaten konfigurieren, sonst kann jeder im Netz mitlesen/publizieren.

---

## Speicher

**Aufteilung nach Ersetzbarkeit** (statt nach Funktion — das ist der Schlüssel für die Backup-Frage):

| Speicher | Verwendung | Ersetzbar? |
|---|---|---|
| 256 GB M.2 SSD (intern, vorhanden) | Ubuntu + Docker-Volumes + alle Dienste + **Nextcloud-Daten (50–100 GB)** | OS ja, Nextcloud **nein** → Backup |
| **8 TB USB 3.0 HDD extern** | Filmbibliothek (Jellyfin) **+ ROMs** (alle Systeme) | ja (neu rippbar/dumpbar, siehe Caveat unten) |
| USB-Stick | Backup (Nextcloud, HA, n8n, AdGuard) + Passwörter, wichtige Dokumente | unersetzlich → offline |

> **SSD-Verschleiß durch DB-Workload:** MariaDB (Nextcloud) schreibt laufend kleine Änderungen — bei dieser Nutzergröße (Einzelnutzer, 50–100 GB) unkritisch, aber gelegentlich NVMe-Health prüfen: `smartctl -a /dev/nvme0`, Attribut `Percentage Used` im Blick behalten.

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

> **Caveat "jederzeit dumpbar":** Gilt nur, solange die Original-Module/Discs noch vorhanden sind. Falls ein Original nicht mehr vorhanden ist (verkauft, verloren, defekt), ist der zugehörige ROM-Dump faktisch unersetzlich und sollte dann ins Backup-Konzept aufgenommen werden statt als "eh neu erzeugbar" zu gelten.

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

### Übergangslösung: vorhandene Toshiba DT01ACA200 (2 TB)

Bereits vorhandene 3,5"-Desktop-Platte (Toshiba DT01ACA200, 2 TB, Baujahr Okt. 2016) passt mechanisch/elektrisch exakt ins **Inateck FE3002** (bereits vorhanden, s. Einkaufsliste). Kann als **Zwischenlösung** dienen, bis Option A/B (8 TB CMR) gekauft ist — z. B. um Jellyfin/Samba-Setup zu testen.

- **Vor Einsatz:** SMART-Werte prüfen (`smartctl -a /dev/sdX`) — `Reallocated_Sector_Ct`, `Current_Pending_Sector`, `Offline_Uncorrectable` müssen 0 sein, `Power_On_Hours` deutlich unter 40.000 h.
- **Kein 24/7-Dauerbetrieb** — Consumer-Desktop-Baureihe (DT01ACA), nicht auf Dauerlast spezifiziert, zusätzlich schon ~10 Jahre alt.
- **Nur für ersetzbare/unkritische Daten** (Test-Setup, ein paar ROMs/Filme) — keine Nextcloud- oder sonst unersetzlichen Daten drauf.
- Nur 2 TB — reicht nicht als Dauerlösung für das Zielbild (Filme + ROMs), nur als Brücke.

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

Backup nur für das **Unersetzliche** — Filme und ROMs sind ersetzbar (neu rippbar/dumpbar, siehe Caveat unten), der Rest der Config-/Nutzdaten auf der internen SSD nicht:

| Was | Backup wie |
|---|---|
| Filmbibliothek + ROMs (8 TB extern) | kein separates Backup — neu rippbar/dumpbar |
| **Nextcloud-Daten + -Datenbank (auf SSD, 50–100 GB)** | automatisiert + versioniert auf USB-Stick |
| Home-Assistant-Config, n8n-Workflow-Export, AdGuard-Config | automatisiert + versioniert auf USB-Stick |
| Dokumente / Passwörter | USB-Stick, räumlich getrennt aufbewahren |

**Automatisiert + versioniert, ohne neue Hardware:** `systemd`-Timer (täglich) + `rsync` mit Hardlink-Versionierung auf den vorhandenen USB-Stick — funktioniert wie Time Machine, mehrere Zeitstände dank Hardlinks, nur tatsächlich geänderte Dateien kosten echten Zusatzplatz:

```bash
rsync -a --link-dest=<letzter-snapshot> <quelle> <ziel>/<timestamp>
```

**Konsistenter Nextcloud-Restore:** Vor jedem rsync-Lauf die Datenbank sauber sichern, statt nur die Dateien im laufenden Betrieb zu kopieren:

```bash
docker exec nextcloud php occ maintenance:mode --on
docker exec nextcloud-db mariadb-dump ... > nextcloud-db-<timestamp>.sql
docker exec nextcloud php occ maintenance:mode --off
```

**USB-Stick nur während des Backup-Laufs angesteckt** — danach abziehen und getrennt lagern (nicht dauerhaft am Server). Schützt das Backup vor Diebstahl, Brand oder Überspannung, die gemeinsam mit dem Server auftreten könnten.

**Restore-Test als einmaliger Schritt nach Ersteinrichtung:** Testweise auf einen Nebenordner zurückspielen und verifizieren, dass die Daten lesbar und vollständig sind — bevor man sich auf das Backup verlässt.

**Kapazität neu durchgerechnet:** Nextcloud-Daten+DB (50–100 GB) + HA/n8n/AdGuard-Configs (wenige GB) + Snapshot-Historie durch Hardlink-Versionierung (nur geänderte Dateien kosten echten Zusatzplatz, aber bei aktiver Nextcloud-Nutzung realistisch 1–5 GB/Woche) — ein 128-GB-Stick wird damit zu knapp, **256 GB** ist die richtige Größe (siehe Einkaufsliste). Um das Wachstum zu begrenzen: Snapshots älter als 14 Tage im selben `systemd`-Timer automatisch löschen (`find <ziel> -maxdepth 1 -mtime +14 -exec rm -rf {} \;`), sonst wächst die Historie unbegrenzt weiter.

**Vorheriger "blinder Fleck" jetzt geschlossen:** Home-Assistant-Automationen, n8n-Workflows und AdGuard-Custom-Filterlisten liefen bisher ungesichert nur auf der internen SSD. Das oben erweiterte Backup deckt sie jetzt mit ab — kein Mehraufwand, da ohnehin automatisiert. Verbleibendes Restrisiko: kein Hardware-Mirror für die SSD selbst (z. B. RAID) — das wird bewusst akzeptiert, da das OS im Fehlerfall neu installierbar ist und jetzt alle unersetzlichen Daten gesichert sind.

---

## Netzwerk & Remote-Zugriff

- **LAN:** Gigabit-Kabel bevorzugt — nativer LAN-Port hat einen defekten Pin (deshalb aktuell kein Wake-on-LAN möglich, Reparatur evtl. später), USB 3.0 Gigabit Adapter (~15€) als Dauerlösung. Realer Durchsatz ~700–900 Mbps, für Streaming und alle Dienste völlig ausreichend.
- **SSH:** direkt im lokalen Netz, kein Extra-Setup nötig

### Gäste-Netzwerk / Segmentierung

Router unterstützt Gäste-WLAN/VLAN — wird eingerichtet. Normale Hausgäste nutzen dieses separate WLAN, das keinen Zugriff auf Admin-Oberflächen (AdGuard-UI, Home-Assistant-UI, n8n-UI, Samba) im Hauptnetz hat. Kein Zugriff aufs Dienste-Subnetz nötig, da die Jellyfin-Freigabe für Freunde/Familie ohnehin extern über Cloudflare Tunnel läuft (siehe unten), nicht übers Gäste-WLAN.

### USB-Bandbreite: externe HDD + LAN-Adapter

Beide hängen am selben Bus-System. Nach Möglichkeit externe 8-TB-HDD und USB-LAN-Adapter auf unterschiedliche physische USB-Controller/Root-Hubs verteilen — beim Micro-Formfaktor oft vorne/hinten getrennte Controller. Nach der Erstinstallation mit `lsusb -t` prüfen, welche Ports auf demselben Controller hängen, bei Bedarf umstecken.

### Remote-Zugriff: Tailscale (empfohlen)

Tailscale erstellt ein privates WireGuard-Mesh-Netzwerk zwischen eigenen Geräten. Kein Port-Forwarding, funktioniert hinter CGNAT, keine feste IP nötig.

- Installation auf OptiPlex + alle Client-Geräte (Handy, Laptop)
- Jedes Gerät bekommt eine stabile interne IP (`100.x.x.x`)
- Jellyfin, Nextcloud etc. erreichbar als wären sie im Heimnetz
- Kostenlos: 6 Nutzer/Tailnet, unbegrenzte Geräte (Stand Juli 2026)

> Tailscale erfordert die App auf jedem Client-Gerät. Wer Dienste ohne App für andere teilen möchte → Cloudflare Tunnel als Alternative.

### HTTPS für Nextcloud über Tailscale — entschieden

Nextcloud braucht valides HTTPS, sonst brechen WebDAV, Zwischenablage-Sync und einzelne Mobile-Client-Features. Die reine Tailscale-IP liefert das nicht automatisch. Lösung: Tailscale kann selbst echte, öffentlich vertrauenswürdige Let's-Encrypt-Zertifikate fürs eigene Tailnet ausstellen.

**Setup:**
1. Im Tailscale-Admin-Panel unter "HTTPS Certificates" aktivieren
2. Auf dem Server: `tailscale cert optiplex.<tailnet-name>.ts.net` → erzeugt Zertifikat + Key
3. Zertifikat an einen davorgeschalteten Reverse-Proxy übergeben — **Caddy**

**Entschieden: Caddy als eigener Container im `homelab_net`.** Nextcloud selbst lauscht intern nur auf Port 80, nicht 443 — ein direktes `tailscale serve https / http://localhost:443` funktioniert deshalb nicht (Port-Mismatch, Nextcloud hört dort gar nicht). Caddy übernimmt die TLS-Terminierung extern auf 443 mit dem von `tailscale cert` ausgestellten Zertifikat und reicht intern an `nextcloud:80` weiter — minimale Config, automatisches Zertifikats-Handling für alles Weitere.

```
optiplex.<tailnet-name>.ts.net {
    tls /pfad/zu/cert.crt /pfad/zu/cert.key
    reverse_proxy nextcloud:80
}
```

Zusätzlich in Nextclouds `config.php` (bzw. per Env-Variablen im `nextcloud:apache`-Image) setzen, sonst meldet Nextcloud falsche URLs/blockt Zugriffe:
- `trusted_domains` → der Tailscale-Hostname (`optiplex.<tailnet-name>.ts.net`)
- `trusted_proxies` → die Container-IP von Caddy im `homelab_net`
- `overwriteprotocol` → `'https'`

Damit ist `https://optiplex.<tailnet-name>.ts.net` von jedem Tailnet-Gerät mit gültigem Schloss-Symbol erreichbar.

Betrifft nur den Tailscale-Zugriffspfad — Jellyfin über Cloudflare Tunnel hat TLS bereits automatisch über die Cloudflare-Edge gelöst.

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
- **Tailscale** für den eigenen dauerhaften Zugriff (Nextcloud, Home Assistant, n8n, SSH) und für den **Kollegen-Zugang zum Trading-Bot** — eigene ACL-Regel muss sowohl den Bot-Port (`dst: bot-host:bot-port`) als auch SSH (`dst: bot-host:22`) für den Kollegen-Host freigeben, nicht nur einen einzelnen Port, sonst kommt er gar nicht erst per SSH auf den Host. Zusätzlich `ForceCommand`/rbash im Container selbst als Defense-in-Depth (siehe "Gastzugang für Kollegen"). Kein öffentlicher Angriffspunkt, Ende-zu-Ende verschlüsselt.
- **Cloudflare Tunnel + Access (E-Mail-OTP)** nur für **Jellyfin-Freigabe an Freunde/Familie**, die keinen eigenen Tailscale-Client installieren sollen. Tailscale Funnel wäre die naheliegende Alternative, ist aber noch Beta mit undokumentierten Limits — Cloudflares E-Mail-OTP-Zugang ist ausgereifter und pro E-Mail-Adresse granular steuerbar. Dafür einen dedizierten, eingeschränkten Jellyfin-Benutzer-Account für externe Gäste anlegen (kein Admin-Zugriff, keine Bibliotheks-/Server-Verwaltung), und die Cloudflare-Access-Policy so konfigurieren, dass wirklich nur der Jellyfin-Player-Pfad erreichbar ist — nicht versehentlich weitere interne Dienste über denselben Tunnel-Hostnamen mitexponiert werden.

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
| Festplattengehäuse (3,5", USB) | Inateck FE3002 | vorhanden |

### Noch zu kaufen

| Was | Warum | ~Kosten |
|---|---|---|
| USB 3.0 Gigabit LAN-Adapter | Nativer LAN-Port hat defekten Pin (kein Wake-on-LAN möglich, Reparatur evtl. später) | ~15 € |
| **8 TB extern** — Option A: Seagate Exos 7E8 8TB `ST8000NM0055` (gebraucht, SMART prüfen) + Inateck FE3002 Gehäuse · Option B: Seagate IronWolf 8TB `ST8000VN004` (neu) + Inateck FE3002 | Filme (Jellyfin) **+ ROMs** | ~107–125 € (A) / ~342 € (B) |
| **Externes USB Blu-ray-Laufwerk** | Discs rippen mit MakeMKV | ~60–80 € |
| USB-Stick (256 GB) | Backup (Nextcloud-Daten+DB 50–100 GB, HA/n8n/AdGuard-Configs, Snapshot-Historie) + wichtige Dateien — 128 GB reicht mit der jetzt hinzugekommenen Versionierung zu knapp | ~20–30 € |
| **Gamepad** — 8BitDo Pro 2 (SNES-Layout, 2D-Fokus) oder Ultimate 2 (Xbox-Layout, Analog-Fokus) | Emulation | ~45–60 € |

> **Bewusst nicht gekauft:**
> - *Interne 2,5"-HDD* — überteuert (~94–150 € für 2 TB) und überflüssig, da Nextcloud auf die SSD passt und ROMs auf die externe Platte gehen.
> - *4-TB-Externe* — lohnt nicht: ausnahmslos SMR, teils teurer als die 8-TB-CMR. Direkt 8 TB nehmen.
>
> **Optional (nur bei Bedarf):**
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

## Gastzugang für Kollegen (Trading Bot, optional / zurückgestellt)

> Hängt am Trading-Bot-Status oben — nur relevant, falls der Bot doch umgesetzt wird.

Sicherer Zugang für eine Vertrauensperson die nur am Trading Bot arbeiten soll — kein Zugang zu anderen Diensten, kein sudo auf das System.

### Voraussetzung: Tailscale

Kollege installiert Tailscale, du lädst ihn per E-Mail ins Tailnet ein. Er erreicht den OptiPlex dann über die Tailscale-IP — keine offenen Ports, kein VPN-Gebastel.

### Warum der Compose-Pfad root-only sein muss

Läge die `docker-compose.yml` für den Trading Bot im Home-Verzeichnis des Kollegen, könnte er sie beliebig überschreiben — z. B. um `privileged: true` einzuschmuggeln und darüber vollen Host-Root-Zugriff zu erlangen, obwohl "kein sudo auf das System" explizit das Ziel ist. Deshalb liegt die Datei in einem root-only-Pfad, den der Kollege nur lesen, nicht schreiben kann, und die Sudoers-Regel zeigt exakt auf diesen Pfad.

### Einrichtung

```bash
# 1. Dedizierter User — Shell wird unten per ForceCommand erzwungen, nicht per rbash
sudo useradd -m -s /bin/bash kollege
sudo passwd kollege

# 2. SSH-Key des Kollegen hinterlegen
sudo mkdir -p /home/kollege/.ssh
sudo nano /home/kollege/.ssh/authorized_keys
# → Public Key des Kollegen einfügen
sudo chown -R kollege:kollege /home/kollege/.ssh
sudo chmod 700 /home/kollege/.ssh

# 3. Compose-Datei in root-only-Pfad, NICHT im Home-Verzeichnis des Kollegen
sudo mkdir -p /opt/trading-bot
sudo nano /opt/trading-bot/docker-compose.yml
sudo chown root:root /opt/trading-bot/docker-compose.yml
sudo chmod 644 /opt/trading-bot/docker-compose.yml   # Kollege kann lesen, nicht schreiben

# 4. Sudoers-Regel — zeigt exakt auf den root-only-Pfad
sudo nano /etc/sudoers.d/kollege
```

```
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker restart trading-bot
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker logs trading-bot
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker compose -f /opt/trading-bot/docker-compose.yml up -d
kollege ALL=(ALL) NOPASSWD: /usr/bin/docker compose -f /opt/trading-bot/docker-compose.yml down
```

### Restricted Shell statt normalem Bash

`/bin/bash` als Login-Shell würde dem Kollegen freie Kommandoausführung erlauben — nicht ausreichend eingeschränkt für "kein Zugang zu anderen Diensten".

**Entschieden: `ForceCommand` statt `rbash`.** `rbash` hat bekannte Escape-Wege über Programme, die selbst eine Shell öffnen können (z. B. `vi`, `awk`, `more`) — ein Nutzer mit `docker`-Zugriff findet darüber leicht einen Ausbruch. `ForceCommand` in `/etc/ssh/sshd_config` erzwingt dagegen unabhängig von der Login-Shell exakt ein definiertes Skript, das nur `scp` in einen festgelegten Unterordner sowie die erlaubten `docker`-Befehle durchlässt:

```
Match User kollege
    ForceCommand /usr/local/bin/kollege-restricted-shell.sh
```

`/usr/local/bin/kollege-restricted-shell.sh` prüft `$SSH_ORIGINAL_COMMAND` gegen eine Allowlist (nur die vier Sudoers-Docker-Befehle + `scp` nach `~/trading-bot/`) und lehnt alles andere ab.

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

## KI-gestützte Umsetzung

**Grundidee:** Bei ~80% des Stacks fehlt die Detail-Wissensmenge (nicht das Know-how), und die Zeit zum Aneignen ist bewusst nicht investiert. Die Dokumentation soll deshalb so aufbereitet sein, dass eine KI-Session das Projekt **allein aus diesen Dateien heraus** korrekt umsetzen kann — Entscheidungen respektieren, Gotchas kennen, Ergebnis selbst verifizieren — ohne dass währenddessen Live-Zugriff auf den Server nötig ist. (Live-Ops-Zugriff — Claude schaut per SSH auf den laufenden Server — ist eine mögliche spätere Ausbaustufe, aber bewusst **nicht** der aktuelle Fokus.)

**Drei Dokumente für KI-Lesbarkeit:**

1. **[`SETUP-PRP.md`](./SETUP-PRP.md)** — der Einrichtungsplan im "Product Requirement Prompt"-Format: Ziel, Kontext (alle Gotchas an einem Ort statt über die README verteilt), Schritt-für-Schritt-Blueprint, und eine **Validation Loop** — pro Schritt ein konkret ausführbares Kommando, mit dem geprüft wird, ob er erfolgreich war. Damit kann eine KI-Session den Aufbau eigenständig durchführen und selbst gegenchecken, statt nur Prosa-Anweisungen zu befolgen.
2. **[`llms.txt`](./llms.txt)** — schlanker Index nach dem [llms.txt-Standard](https://llmstxt.org): sagt einer KI-Session in ein paar Zeilen, welche Datei wofür da ist (Entscheidungen & Recherche vs. Ausführungsplan vs. Arbeitskonventionen), statt dass bei jeder Anfrage die komplette README geladen werden muss.
3. **`CLAUDE.md`** — projekt-spezifische Arbeitskonventionen für KI-Sessions in diesem Ordner (wird von Claude Code automatisch gelesen): wie mit bereits getroffenen Entscheidungen umzugehen ist, welcher Status (`geplant`/`optional`/`entschieden`) was bedeutet, worauf beim Vorschlagen von Änderungen zu achten ist.

**Weitergedacht, optional:**
- **n8n als KI-Automatisierungs-Hub** — native Claude/Anthropic-API-Nodes ermöglichen z. B. automatisierte Log-Zusammenfassungen oder Uptime-Kuma-Alert-Triage. Setzt laufenden Betrieb voraus, also erst relevant, wenn der Server steht.
- **Live-Ops-Zugriff** (Claude per SSH direkt am laufenden Server) — bewusst zurückgestellt, siehe oben. Falls später gewünscht: SSH-Key-Zugriff über Tailscale + Ausbau von `CLAUDE.md` um Host/Compose-Pfade.
- **Home Assistant + KI-Sprachsteuerung** — vermutlich über Community-Integrationen möglich, aber Kompatibilität ändert sich schnell und wurde nicht verifiziert — nur als Idee vorgemerkt.

---

## Offene Fragen

- [x] **Desktop-Umgebung**: KDE Plasma (Kubuntu 26.04 LTS) — siehe Abschnitt "Betriebssystem: Linux"
- [x] **Remote-Zugriff**: Hybrid — Tailscale (eigener Zugriff + Kollege) + Cloudflare Tunnel (Jellyfin-Freigabe an Freunde/Familie), siehe Abschnitt "Netzwerk & Remote-Zugriff"
- [x] **Netzwerk**: USB-Gigabit-LAN-Adapter, da nativer LAN-Port einen defekten Pin hat (deshalb aktuell kein Wake-on-LAN möglich, Reparatur evtl. später) — siehe Abschnitt "Netzwerk & Remote-Zugriff"
- [x] **Nextcloud vs. Syncthing**: Nextcloud — siehe Abschnitt "Lokale Cloud: Nextcloud" (aktuell Einzelnutzer, Mehrbenutzerbetrieb später möglich)
- [ ] **Blu-ray-Laufwerk**: Welches Modell? → MakeMKV-Kompatibilitätsliste prüfen vor dem Kauf
