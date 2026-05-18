# Homelab — Netzwerk & Remote-Zugriff

## Das Grundproblem: CGNAT bei deutschen Kabelanbietern

Praktisch alle deutschen Kabel-ISPs (Vodafone Kabel, Pyur, ...) nutzen heute **DS-Lite**:
Du bekommst eine echte native IPv6-Adresse, aber IPv4 läuft durch Carrier-Grade NAT (CGNAT).
Klassisches Port-Forwarding über IPv4 funktioniert damit nicht.

→ Die Lösung ist nicht "besserer Vertrag" suchen, sondern das Setup CGNAT-agnostisch bauen.

---

## Vertrag — Checkliste (Coax/Kabel)

### Zwingend fragen vor Vertragsabschluss

| Frage | Was du hören willst |
|---|---|
| "Ist es DS-Lite oder DualStack?" | **DualStack** (= echte public IPv4, kein CGNAT) |
| "Kann ich eine statische IPv4 dazubuchen?" | Ja, ~3–5 €/Monat extra |
| "Welchen Upload bekomme ich?" | **≥ 50 Mbit/s**, besser 100 Mbit/s (für Nextcloud-Sync) |
| "Gibt es Port-Sperren auf 80, 443, 8883?" | Nein |

### Anbieter-Einschätzung (Coax, Stand 2026)

| Anbieter | IPv4 | Upoad-Max | Notiz |
|---|---|---|---|
| **Vodafone Kabel** | DS-Lite by default — explizit DualStack anfragen | 100 Mbit/s (GigaCable Max) | Größtes Netz, explizit nachfragen |
| **NetCologne** | DualStack, echte IPv4 | 300 Mbit/s | Nur Köln/Bonn/NRW-Raum, empfohlen wenn verfügbar |
| **M-Net** | DualStack | 100 Mbit/s | Nur Bayern/München |
| **Pyur** | CGNAT auf vielen Leitungen | 50 Mbit/s | Nicht empfohlen für Homelab |
| **Wilhelm.tel** | DualStack | 200 Mbit/s | Nur Norderstedt/Hamburg Umland |

> **Tipp:** Beim Anruf direkt fragen "Bekomme ich eine öffentliche IPv4-Adresse (DualStack) oder DS-Lite/CGNAT?". Wenn der Support nicht antworten kann: Abstand nehmen.

### Upload ist wichtiger als Download für Nextcloud

Kabel ist asymmetrisch. 1 Gbit/s Download klingt gut, aber wenn du von unterwegs 4K-Fotos in Nextcloud synchst, limitiert der Upload. Ziel: **≥ 50 Mbit/s Upload**, bei Vodafone Kabel bisher max. 100 Mbit/s auf GigaCable Max.

---

## Router-Setup

### Aktuelle Situation

FritzBox 7520 = DOCSIS-Kabelmodem + Router kombiniert. Kein echter Bridge-Mode → bleibt als kombiniertes Gerät erhalten.

### Phase 1 — Sofort einsatzbereit (FritzBox 7520 behalten)

```
[Kabel-ISP] → [FritzBox 7520] → [OptiPlex 3090]
                                 └─ Tailscale
                                 └─ Port-Freigaben (wenn DualStack)
```

Einrichten:
1. **FritzOS aktualisieren** → auf Version 7.50+ für WireGuard-Support
2. **MyFRITZ! aktivieren** → kostenloser DynDNS von AVM, automatisch (myfritz.net-Subdomain)
3. **WireGuard-Server in FritzBox** aktivieren → für direkten VPN-Einwahl (Handy, Laptop)
4. **Port-Freigaben** eintragen (nur wenn DualStack/echte IPv4):
   - 443 → OptiPlex (Nextcloud via Caddy)
   - 8883 → OptiPlex (MQTT/TLS für Companion Watch)
5. **Tailscale** auf OptiPlex installieren → als primärer Remote-Zugriff (funktioniert auch hinter CGNAT)

**Wenn DS-Lite (kein echtes IPv4):** Tailscale + Hetzner VPS übernehmen, Port-Freigaben entfallen.

### Phase 2 — Sauberes Homelab-Netz (optional, ~230 €)

FritzBox 7520 kann kein VLAN-Routing. Für saubere Netztrennung (IoT ↔ Server ↔ Heimnetz):

**Neue Hardware:**

| Gerät | Modell | Preis | Rolle |
|---|---|---|---|
| DOCSIS 3.1 Kabelmodem | ARRIS S33 oder Motorola MB8611 | ~80 € | Nur Modem, echter Bridge-Mode |
| Router/Firewall | N100 Mini-PC (z.B. Beelink EQ12 Pro) mit OPNsense | ~150 € | Routing, VLANs, Firewall, WireGuard |
| Managed Switch | Netgear GS308E (8-Port) | ~40 € | VLAN-Verteilung im LAN |

**Netzwerk-Architektur Phase 2:**

```
[Kabel-ISP]
    ↓
[DOCSIS-Modem] ← Bridge-Mode, reine Umwandlung Coax→Ethernet
    ↓ öffentliche IPv4 (bei DualStack) oder IPv6-native (bei DS-Lite)
[OPNsense Box]
    ├─ VLAN 10: Heimnetz  (192.168.10.0/24) ← PCs, Handys, Piano-Display (RPi)
    ├─ VLAN 20: Server    (192.168.20.0/24) ← OptiPlex 3090
    └─ VLAN 30: IoT       (192.168.30.0/24) ← ESP32 (Companion Watch, Bewässerung)
          ↕ Firewall-Regel: IoT → MQTT auf Server erlaubt, alles andere blockiert
[Managed Switch] ← verteilt VLAN-Tags auf Ports
[WiFi AP] ← separater AP mit SSID pro VLAN (oder FritzBox als reiner AP im VLAN)
```

**Warum VLANs:** IoT-Geräte (ESP32s) haben begrenzte Sicherheits-Updates. VLANs verhindern, dass ein kompromittierter ESP32 deinen Heimnetz-Traffic sehen kann. Server-VLAN isoliert den OptiPlex vom Heimnetz.

---

## Remote-Zugriff — Entscheidungsbaum

```
Willst du von außen auf Nextcloud/HA/SSH zugreifen?
    ↓
    → Tailscale auf OptiPlex installieren.
      Fertig. Läuft hinter CGNAT, hinter doppeltem NAT, überall.
      Keine offene Ports. Kein DynDNS nötig.

Zusätzlich: Companion Watch braucht MQTT von unterwegs?
    ↓
    → ESP32 kann kein Tailscale.
      Lösung: Hetzner VPS als Relay.

Willst du Nextcloud-Links mit anderen teilen (ohne VPN)?
    ↓
    → Auch über Hetzner VPS, oder:
      Bei DualStack: Port 443 auf FritzBox freigeben + Caddy auf OptiPlex.
```

---

## Hetzner VPS als Relay (4 €/Monat)

Für alles was öffentlich erreichbar sein muss ohne Tailscale-Client (ESP32, externe Shares).

**Setup:**

```
[Hetzner CX22 VPS] ← feste public IPv4
    ├─ Caddy (Reverse Proxy, automatisch HTTPS)
    ├─ WireGuard (Tunnel zum OptiPlex)
    └─ MQTT-Port 8883 (TLS) → weiterleiten an OptiPlex via WireGuard

[OptiPlex 3090]
    └─ WireGuard Client → Verbindung zum VPS aufrecht halten
       (funktioniert auch hinter CGNAT, da outbound-initiated)
```

**Was läuft über den VPS:**
- MQTT/TLS (Port 8883) → für Companion Watch und Bewässerung wenn unterwegs
- Nextcloud public share links (optional)
- Kein Video-Streaming über VPS (Bandbreiten-Limit, nur 20 TB/Monat inkl. auf CX22)

**Was läuft direkt über Tailscale (nicht über VPS):**
- Alle persönlichen Nextcloud-Zugriffe
- Home Assistant
- n8n, AdGuard Admin
- SSH, alle Admin-Interfaces

**Gesamtkosten: ~4 €/Monat** (Hetzner CX22)

---

## AdGuard Home als Netz-DNS

AdGuard Home läuft im Docker auf dem OptiPlex und ist DNS für alle Netzgeräte:
- Im Router/DHCP: DNS-Server → `192.168.20.X` (OptiPlex Server-VLAN-IP)
- Alle Geräte im Netz nutzen damit den lokalen Blocker
- Upstream: Cloudflare 1.1.1.1 (DoH) oder Quad9

---

## Offene Punkte (Netzwerk)

- [ ] Vertrag: DualStack vs. DS-Lite beim neuen Anbieter klären und dokumentieren
- [ ] Phase 1 umsetzen: MyFRITZ + WireGuard auf FritzBox, Tailscale auf OptiPlex
- [ ] Hetzner VPS aufsetzen sobald Companion Watch MQTT braucht
- [ ] Phase 2 Hardware kaufen wenn VLAN-Bedarf konkret wird (frühestens wenn IoT-Projekte laufen)
