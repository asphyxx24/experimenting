# PRP: Homelab-Einrichtung (Dell OptiPlex 3090 Micro)

> Product Requirement Prompt — ausführbarer Einrichtungsplan. Enthält alles, was zur eigenständigen Umsetzung nötig ist: Ziel, Kontext/Gotchas, Schritt-für-Schritt-Blueprint, Validation Loop. Hintergrund/Recherche/Entscheidungsbegründungen stehen in `README.md` — hier nur das, was zur Ausführung gebraucht wird.

## Goal

Aus dem Dell OptiPlex 3090 Micro (aktuell Windows 11, Werkszustand) einen laufenden Linux-Homeserver mit allen in der README als `geplant` markierten Diensten machen — betriebsbereit, aber **kein 24/7-Dauerbetrieb**.

## Why

Ersetzt mehrere Einzellösungen (Cloud-Abo, externe NAS, separate Emulations-Kiste) durch einen einzigen sparsamen, selbstverwalteten Knotenpunkt — Kernmotivation Datenhoheit (siehe README → "Lokale Cloud: Nextcloud").

## Success Criteria

- [ ] `homelab_net` angelegt, alle Compose-Dateien binden sich extern daran an
- [ ] Kubuntu 26.04 LTS läuft, Windows ersetzt
- [ ] Docker + `intel-media-va-driver` installiert
- [ ] AdGuard Home läuft, blockt Werbung netzwerkweit, Router hat Fallback-DNS, Docker-DNS-Bypass gefixt, Firewall aktiv
- [ ] Jellyfin läuft, spielt H.265 1080p per Direct Play, Quick-Sync-Transcoding funktioniert (inkl. `render`-GID-Mapping)
- [ ] Externe 8-TB-HDD dauerhaft eingebunden, übersteht Reboot ohne manuelles Eingreifen, USB-Controller-Verteilung geprüft
- [ ] Samba-Freigabe vom Client aus erreichbar
- [ ] Nextcloud (Plain-Setup) läuft, Cron statt AJAX, per Tailscale + Caddy mit validem HTTPS erreichbar
- [ ] Mosquitto (mit Auth) + Home Assistant laufen, ESP32-Projekte können sich verbinden
- [ ] n8n läuft
- [ ] Uptime Kuma überwacht alle obigen Dienste
- [ ] Tailscale + Cloudflare Tunnel eingerichtet, Jellyfin für Freunde freigebbar (eingeschränkter Gast-Account, Access-Policy nur auf Jellyfin-Pfad)
- [ ] Backup eingerichtet (rsync-Hardlink-Versionierung + DB-Dump), Restore-Test erfolgreich
- [ ] PCSX2 + RetroArch nativ installiert, mind. ein Spiel pro Kernsystem läuft

Trading Bot: **nicht** Teil dieses PRP (Status: optional/zurückgestellt, siehe README).

## Context — Gotchas, die beim Ausführen zuschlagen

Diese Punkte sind bereits recherchiert und in der README verteilt dokumentiert — hier zentral gesammelt, weil sie beim blinden Abarbeiten der Schritte sonst zu Fehlern führen:

| # | Gotcha | Betrifft Schritt | Fix | README-Abschnitt |
|---|---|---|---|---|
| 0 | Container ohne gemeinsames Netzwerk erreichen sich nicht per Containername (Home Assistant ↔ Mosquitto, Uptime Kuma ↔ alle) | vor allen Diensten | `docker network create homelab_net` als erster Schritt, jede `docker-compose.yml` bekommt `networks: {default: {name: homelab_net, external: true}}` | "Software-Stack" |
| 1 | Ubuntu/Kubuntu bringt `systemd-resolved` mit, das Port 53 belegt — kollidiert mit AdGuard Home | AdGuard-Setup | `DNSStubListener=no` in `/etc/systemd/resolved.conf`, `systemctl restart systemd-resolved`, **vor** Container-Start | "Einrichtungsreihenfolge" Schritt 6 |
| 2 | AdGuard sieht ohne `network_mode: host` nur die Docker-Gateway-IP statt echter Client-IPs | AdGuard-Setup | `network_mode: host` in `docker-compose.yml` | "Einrichtungsreihenfolge" Schritt 6 |
| 2b | Andere Container nutzen Dockers internen DNS-Resolver und umgehen AdGuard | AdGuard-Setup | `/etc/docker/daemon.json`: `{"dns": ["<AdGuard-Host-IP>"]}`, Docker-Daemon neu starten; alternativ `dns:` pro Compose-Datei | "Einrichtungsreihenfolge" Schritt 6 |
| 2c | `network_mode: host` hebt die normale Docker-Netzwerkisolation für AdGuard auf | AdGuard-Setup | `ufw`/`nftables`: nur Port 53 + Web-UI-Port fürs LAN-Subnetz freigeben, Rest blocken | "Einrichtungsreihenfolge" Schritt 6 |
| 3 | Nextcloud AIO frisst dokumentiert 25–34 GB RAM statt 4–5 GB (Elasticsearch/ClamAV/Collabora-Container) | Nextcloud-Setup | **Plain** `nextcloud:apache`-Image + eigene MariaDB + Redis verwenden, nicht AIO | "Lokale Cloud: Nextcloud" |
| 3b | Nextcloud-Hintergrundaufgaben (Vorschaubilder, Scans, Benachrichtigungen) laufen im AJAX-Modus nur unzuverlässig | Nextcloud-Setup | Admin-Einstellungen auf "Cron" umstellen, `systemd`-Timer alle 5 Min.: `docker exec -u www-data nextcloud php occ system:cron` | "Lokale Cloud: Nextcloud" |
| 4 | Ohne `/dev/dri`-Passthrough sieht Jellyfin die iGPU nicht → Software-Transcoding statt Quick Sync | Jellyfin-Setup | `devices: - /dev/dri:/dev/dri` in `docker-compose.yml` + `intel-media-va-driver` installiert | "Jellyfin: Quick Sync auf Linux einrichten" |
| 4b | Trotz sichtbarem `/dev/dri` kann Permission-Denied auftreten, wenn die Container-Gruppe nicht passt | Jellyfin-Setup | `group_add: - "<render-gid>"` in `docker-compose.yml`, GID vorher mit `getent group render` ermitteln | "Jellyfin: Quick Sync auf Linux einrichten" |
| 5 | Externe HDD im Werkszustand ist NTFS/exFAT, kein Journaling, 4-GB-Dateilimit bei exFAT/FAT32 | HDD-Einbindung | Auf **ext4** formatieren | "Linux-Setup der externen Platte" |
| 6 | Ohne `nofail` in `/etc/fstab` bootet der Server nicht, wenn die USB-HDD mal nicht dranhängt | HDD-Einbindung | Per UUID + `nofail`-Option einbinden | "Linux-Setup der externen Platte" |
| 7 | USB-Autosuspend killt die HDD-Verbindung im Dauerbetrieb (I/O-Fehler, spontanes Unmount) | HDD-Einbindung | `usbcore.autosuspend=-1` in `/etc/default/grub`, `update-grub`, Neustart | "Linux-Setup der externen Platte" |
| 8 | Nextcloud über reine Tailscale-IP hat kein valides HTTPS → WebDAV/Mobile-Clients brechen. Nextcloud lauscht intern nur auf Port 80 — ein direktes `tailscale serve https / http://localhost:443` funktioniert wegen Port-Mismatch nicht | Nextcloud + Tailscale | **Caddy**-Container im `homelab_net`, lauscht extern auf 443 mit `tailscale cert`-Zertifikat, `reverse_proxy nextcloud:80`; zusätzlich in Nextcloud `trusted_domains`, `trusted_proxies` (Caddy-Container-IP), `overwriteprotocol => 'https'` setzen | "HTTPS für Nextcloud über Tailscale" |
| 9 | Externe Platte könnte SMR statt CMR sein (bricht bei parallelem Lesen+Schreiben ein) | HDD-Kauf | Nur die in README genannten Modelle kaufen (Seagate Exos/IronWolf), SMART-Werte vor Kauf prüfen bei Gebraucht-Kauf | "Die externe Platte: Zwei Optionen" |
| 10 | Externe HDD und USB-LAN-Adapter am selben USB-Controller können sich die Bandbreite teilen | Netzwerk/HDD-Einbindung | `lsusb -t` nach Erstinstallation prüfen, bei Bedarf auf getrennte Controller/Root-Hubs umstecken | "Netzwerk & Remote-Zugriff" |
| 11 | Mosquitto ohne Auth erlaubt jedem im Netz Mitlesen/Publizieren | Mosquitto-Setup | `allow_anonymous false` in `mosquitto.conf`, Nutzer mit `mosquitto_passwd` anlegen, ESP32-Projekte entsprechend konfigurieren | "MQTT & IoT-Integration" |

## Implementation Blueprint

Reihenfolge ist bewusst so gewählt (Abhängigkeiten beachten — z. B. AdGuard vor allem anderen, da DNS-Grundlage fürs ganze Netz; Docker-Netzwerk vor jedem Dienst; Mosquitto vor Home Assistant, da dessen MQTT-Integration den bereits laufenden Broker braucht):

1. **Docker-Netzwerk anlegen** — `docker network create homelab_net` (Gotcha #0)
2. **Linux installieren** — Kubuntu 26.04 LTS, Windows ersetzen
3. **Grundkonfiguration** — SSH aktivieren, `apt update && apt upgrade`, Benutzer einrichten
4. **Docker installieren** — offizielle Docker-Anleitung für Ubuntu/Debian-Basis
5. **`intel-media-va-driver` installieren** — Voraussetzung für Jellyfin-Quick-Sync (Gotcha #4)
6. **AdGuard Home** — inkl. Fix für Gotcha #1, #2, #2b, #2c, Router-Fallback-DNS setzen
7. **Externe HDD einbinden** — ext4 formatieren, fstab (Gotcha #5–7), USB-Controller-Verteilung prüfen (Gotcha #10), dann erst Jellyfin/Samba draufzeigen lassen
8. **Jellyfin** — VAAPI konfigurieren (Gotcha #4, #4b), HDD als Medienquelle einbinden
9. **Samba** — Netzlaufwerk auf derselben HDD einrichten
10. **Nextcloud** — Plain-Setup (Gotcha #3), Cron statt AJAX (Gotcha #3b), Speicher auf interner SSD
11. **Mosquitto MQTT** — Broker für ESP32-Projekte, Auth einrichten (Gotcha #11)
12. **Home Assistant** — MQTT-Integration gegen den bereits laufenden Mosquitto-Broker
13. **n8n** — Workflow-Automatisierung
14. **Uptime Kuma** — Monitore für alle bisherigen Dienste anlegen
15. **Tailscale** — Remote-Zugriff, danach Caddy + HTTPS-Zertifikat für Nextcloud (Gotcha #8)
16. **Cloudflare Tunnel + Access** — nur für Jellyfin-Freigabe an Freunde/Familie
17. **Backup einrichten** — `rsync`-Hardlink-Versionierung auf USB-Stick, MariaDB-Dump, Restore-Test
18. **Emulation** — PCSX2 (Flatpak) + RetroArch nativ, BIOS-Dateien von eigener Hardware dumpen

## Validation Loop

Nach jedem Schritt gegenchecken, bevor zum nächsten übergegangen wird:

```bash
# Schritt 1: Docker-Netzwerk existiert
docker network inspect homelab_net

# Schritt 4: Docker läuft
docker --version && docker compose version

# Schritt 5: iGPU für VAAPI sichtbar
vainfo | grep -i "va_display" # sollte Intel-Treiber zeigen

# Schritt 6: AdGuard blockt netzwerkweit
dig @<server-ip> doubleclick.net   # sollte 0.0.0.0 oder NXDOMAIN liefern
docker ps | grep adguard            # Container läuft
ss -tulpn | grep :53                # AdGuard hört auf 53, kein Konflikt mit systemd-resolved

# Schritt 7: HDD übersteht Reboot, USB-Controller-Verteilung geprüft
sudo reboot
df -h | grep <mount-point>          # nach Neustart weiterhin gemountet
lsusb -t                            # HDD und LAN-Adapter auf unterschiedlichen Controllern?

# Schritt 8: Jellyfin Quick Sync aktiv
# In Jellyfin-Dashboard → Wiedergabe → aktive Transkodierungen: "hw" statt "sw" im Log

# Schritt 10: Nextcloud erreichbar + kein AIO-RAM-Ausreißer + Cron aktiv
docker stats --no-stream | grep nextcloud   # RAM im erwarteten Bereich (~300–600 MB), nicht GB-Bereich
docker exec -u www-data nextcloud php occ system:cron   # läuft ohne Fehler

# Schritt 14: Uptime Kuma sieht alle Dienste
# Dashboard prüfen: alle Monitore "Up"

# Schritt 15: Nextcloud-HTTPS über Tailscale/Caddy gültig
curl -I https://optiplex.<tailnet-name>.ts.net   # HTTP 200, kein Zertifikatsfehler

# Schritt 17: Backup + Restore-Test
rsync -a --link-dest=<letzter-snapshot> <quelle> <ziel>/<timestamp>
diff -r <quelle> <ziel>/<timestamp>   # auf Nebenordner zurückspielen, Vollständigkeit prüfen

# Schritt 18: Emulation
# PCSX2: ein Testspiel startet, Controller wird über SDL erkannt
```

## Anti-Patterns (aus README-Entscheidungen abgeleitet)

- **Nicht** Nextcloud AIO installieren, auch wenn es "einfacher" wirkt (Gotcha #3)
- **Nicht** die HDD im Werksformat (NTFS/exFAT) belassen
- **Nicht** WD My Book/Easystore kaufen (Hardware-Verschlüsselung, Totalverlust-Risiko bei Controller-Defekt)
- **Nicht** SMR-Platten kaufen (4-TB-Modelle sind ausnahmslos SMR)
- **Nicht** den Trading Bot in diesem Durchlauf umsetzen — Status ist zurückgestellt
- **Nicht** Container-Netzwerke pro Dienst isoliert lassen — alle nutzen das gemeinsame `homelab_net` (Gotcha #0)
- **Nicht** `tailscale serve https / http://localhost:443` für Nextcloud verwenden — Port-Mismatch, Caddy ist die korrekte Lösung (Gotcha #8)
- **Nicht** die Trading-Bot-`docker-compose.yml` im Home-Verzeichnis des Kollegen ablegen — root-only-Pfad (`/opt/trading-bot`) verhindert `privileged: true`-Eskalation
