# Homelab — Netzwerk & Remote-Zugriff

## Klarstellung: FritzBox 7520 und Coax

Die FritzBox 7520 ist ein **DSL-Router (VDSL2/ADSL2+)** — sie kann keine Coax-Leitung verarbeiten.
AVM-Nomenklatur: 7xxx = DSL, 6xxx = Kabel/DOCSIS.

DSL (Telefonleitung, verdrilltes Kupferpaar) und Kabelinternet (Coax, DOCSIS-Protokoll) sind komplett verschiedene Technologien — man kann kein DSL über eine Coax-Dose betreiben.

**Was mit der 7520 passiert:**
- Phase 1: weiterhin als Router hinter einem Kabelmodem betreiben (FritzOS unterstützt "Internet via LAN"-Modus)
- Oder: als reiner WLAN-Access-Point im neuen Setup verwenden
- Oder: verkaufen (~30–50€)

**Der OptiPlex 3090 ist kein Router** — er soll Application Server bleiben:
- Sein einziger Netzwerkanschluss ist ein USB 3.0 Adapter (nativer Port defekt) — ein Router braucht mindestens 2 Interfaces
- Wenn der Server für Updates neustartet, verliert man sonst das Internet
- Klare Trennung: Netzinfrastruktur ↔ Services

---

## Das Grundproblem: CGNAT bei deutschen Kabelanbietern

Praktisch alle deutschen Kabel-ISPs (Vodafone Kabel, Pyur, ...) nutzen heute **DS-Lite**:
Native IPv6 vorhanden, aber IPv4 läuft durch Carrier-Grade NAT (CGNAT).
Klassisches Port-Forwarding über IPv4 funktioniert damit nicht.

→ Das Setup muss CGNAT-agnostisch funktionieren.

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

| Anbieter | IPv4 | Upload-Max | Notiz |
|---|---|---|---|
| **Vodafone Kabel** | DS-Lite by default — explizit DualStack anfragen | 100 Mbit/s (GigaCable Max) | Größtes Netz, explizit nachfragen |
| **NetCologne** | DualStack, echte IPv4 | 300 Mbit/s | Nur Köln/Bonn/NRW-Raum |
| **M-Net** | DualStack | 100 Mbit/s | Nur Bayern/München |
| **Pyur** | CGNAT auf vielen Leitungen | 50 Mbit/s | Nicht empfohlen für Homelab |
| **Wilhelm.tel** | DualStack | 200 Mbit/s | Nur Norderstedt/Hamburg Umland |

> **Tipp:** Direkt fragen: "Bekomme ich eine öffentliche IPv4-Adresse (DualStack) oder DS-Lite/CGNAT?" — wenn der Support die Frage nicht beantworten kann: Abstand nehmen.

### Upload ist wichtiger als Download für Nextcloud

Kabel ist asymmetrisch. Ziel: **≥ 50 Mbit/s Upload**. Bei Vodafone Kabel derzeit max. 100 Mbit/s auf GigaCable Max.

---

## Router-Hardware

### Phase 1 — Schnellstart mit vorhandener FritzBox (~80–100 €)

```
[Coax-Dose]
    ↓
[FritzBox 6660 Cable oder 6591 Cable]   ← DOCSIS 3.1 + Router in einem Gerät
    ↓ LAN
[OptiPlex 3090]
    └─ Tailscale (Remote-Zugriff)
    └─ Port-Freigaben in der FritzBox (wenn DualStack)
```

**Empfohlen: FritzBox 6660 Cable** (~100 € neu, ~60–80 € gebraucht)
- Verbindet sich direkt mit der Coax-Dose (DOCSIS 3.1)
- Bekannte FritzOS-Oberfläche wie die 7520
- WireGuard VPN-Server eingebaut (ab FritzOS 7.50)
- MyFRITZ! DynDNS kostenlos inklusive
- Eingeschränktes VLAN-Routing (reicht für Phase 1)

**Alternativ: FritzBox 7520 weiterverwenden** hinter einem reinen DOCSIS-Modem:
- Separates DOCSIS 3.1 Modem (z.B. ARRIS S33, ~80 €) in Bridge-Mode
- FritzBox 7520 als Router dahinter ("Internet via LAN"-Modus in FritzOS)
- Erspart die Anschaffung eines neuen Routers, aber aufwändiger

### Phase 2 — Sauberes Homelab-Netz (~270 €)

Für VLAN-Trennung (IoT ↔ Server ↔ Heimnetz) braucht man OPNsense:

| Gerät | Modell | Preis | Rolle |
|---|---|---|---|
| DOCSIS 3.1 Modem | ARRIS S33 oder Motorola MB8611 | ~80 € | Nur Modem, Bridge-Mode |
| Router/Firewall | N100 Mini-PC (z.B. Beelink EQ12 Pro) mit OPNsense | ~150 € | Routing, VLANs, Firewall, WireGuard |
| Managed Switch | Netgear GS308E (8-Port) | ~40 € | VLAN-Verteilung im LAN |

**FritzBox 6660/7520:** dann als reiner WLAN-Access-Point in einem anderen Raum verwenden oder verkaufen.

**Netzwerk-Architektur Phase 2:**

```
[Coax-Dose]
    ↓
[DOCSIS 3.1 Modem] ← Bridge-Mode, reine Umwandlung Coax→Ethernet
    ↓
[OPNsense Box (N100)]
    ├─ VLAN 10: Heimnetz  (192.168.10.0/24) ← PCs, Handys, Piano-Display (RPi)
    ├─ VLAN 20: Server    (192.168.20.0/24) ← OptiPlex 3090
    └─ VLAN 30: IoT       (192.168.30.0/24) ← ESP32 (Companion Watch, Bewässerung)
          ↕ Firewall-Regel: IoT darf nur MQTT auf Server, nichts sonst
[Netgear GS308E] ← verteilt VLAN-Tags auf Ports
[FritzBox 6660 als AP] ← WLAN pro SSID/VLAN (optional)
```

**Warum VLANs:** ESP32-Geräte bekommen selten Sicherheits-Updates. VLANs verhindern, dass ein kompromittierter ESP32 Heimnetz-Traffic sehen kann.

---

## Remote-Zugriff

```
Zugriff von außen auf Nextcloud / HA / SSH?
    → Tailscale auf OptiPlex.
      Läuft hinter CGNAT, hinter doppeltem NAT, überall.
      Keine offenen Ports, kein DynDNS nötig.

Companion Watch (ESP32) braucht MQTT von unterwegs?
    → ESP32 kann kein Tailscale.
      Lösung: Hetzner VPS als Relay (4 €/Monat).

Nextcloud-Links mit anderen teilen (ohne VPN)?
    → Über Hetzner VPS.
      Oder bei DualStack: Port 443 im Router freigeben + Caddy auf OptiPlex.
```

---

## Hetzner VPS als Relay (4 €/Monat)

Für alles was ohne Tailscale-Client erreichbar sein muss (ESP32, öffentliche Links).

```
[Hetzner CX22]  ← feste public IPv4
    ├─ Caddy (Reverse Proxy, automatisch HTTPS)
    ├─ WireGuard-Tunnel → OptiPlex (outbound-initiated, klappt hinter CGNAT)
    └─ Port 8883 (MQTT/TLS) → durchleiten via WireGuard an Mosquitto auf OptiPlex
```

**Über VPS:** MQTT für ESP32-Projekte, optionale Nextcloud-Shares
**Über Tailscale direkt:** eigener Nextcloud-Zugriff, HA, n8n, SSH, alle Admin-UIs
**Nicht über VPS:** Video-Streaming (CX22 hat 20 TB/Monat — reicht, aber kein CDN)

---

## AdGuard Home als Netz-DNS

Läuft im Docker auf dem OptiPlex. Im Router/DHCP als DNS-Server eintragen → alle Netzgeräte nutzen automatisch den lokalen Blocker.

---

## Offene Punkte (Netzwerk)

- [ ] Vertrag abschließen: DualStack (echte IPv4) vs. DS-Lite beim Anbieter bestätigen
- [ ] Hardware kaufen: FritzBox 6660 Cable (Phase 1) oder direkt DOCSIS-Modem + OPNsense (Phase 2)
- [ ] FritzBox 7520 Verwendung klären: AP behalten oder verkaufen
- [ ] Tailscale auf OptiPlex einrichten nach Linux-Installation
- [ ] Hetzner VPS aufsetzen sobald Companion Watch MQTT von außen braucht
- [ ] Phase 2 Hardware wenn IoT-Projekte konkret werden
