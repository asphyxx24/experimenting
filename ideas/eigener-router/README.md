# Eigener Router

Eigenes Netzwerk-Setup ohne proprietäre Provider-Hardware (Fritz!Box-Ersatz). Ziel: volle Kontrolle über Firewall, VLANs, DNS — und REST API-Zugriff für Jarvis-Integration.

**Status: Recherche abgeschlossen, Hardware-Kauf ausstehend.**

---

## Kontext

Die Fritz!Box muss zurückgegeben werden. Gleichzeitig besteht Interesse an einem offeneren Setup mit API-Zugriff für die Jarvis-KI. Routerfreiheit in Deutschland ist seit 2016 gesetzlich verankert (§ 41b TKG) — Provider müssen Zugangsdaten herausgeben.

---

## Internetanschluss

**Pyur Kabel (Dresden)** — Koaxialdose bereits im Zimmer vorhanden.

| Tarif | Preis | Dauerhaft |
|---|---|---|
| 250 Mbit/s | 28 €/Monat | ✅ auch ab Monat 25 |
| **500 Mbit/s** | **30 €/Monat** | ✅ auch ab Monat 25 |

Anschlusskosten: 19,99 € einmalig. Der Aufpreis von 2 €/Monat für 500 statt 250 Mbit/s ist minimal — 500 Mbit/s empfohlen.

**Cashback-Deals** (z.B. über Check24/Idealo) sind legitim — der Vertrag läuft direkt mit Pyur, der Portal-Anbieter gibt einen Teil seiner Vermittlungsprovision weiter. Bedingungen prüfen: Wann wird ausgezahlt? Muss aktiv beantragt werden?

---

## Warum kein Kombi-Gerät (Fritz!Box Cable)?

- Proprietäre Firmware, eingeschränkte API (nur TR-064)
- Kein Zugriff auf tiefe Firewall-Regeln, VLANs, VPN-Konfiguration
- Kein REST API für Jarvis-Integration

---

## Warum kein OPNsense?

OPNsense wäre die mächtigste Lösung (vollständige REST API, Suricata IDS/IPS, Plugins), aber:
- Braucht separate Hardware (~150–180 € für Topton/CWWK N100 Box)
- OptiPlex 3090 eignet sich nicht als Router (defekter LAN-Port, Single Point of Failure, bereits für andere Dienste verplant)
- Für den geplanten Use Case Overkill

---

## Entschiedene Lösung

### Router: MikroTik hAP ac2 (gebraucht, ~40–60 €)

- WiFi 5, 5x Gigabit LAN, lüfterlos
- RouterOS 7 → vollständige REST API (`https://<ip>/rest/`) für Jarvis
- Schafft 250–500 Mbit/s mit NAT/Firewall problemlos
- 256 MB RAM — für Heimnetz mit ~10–20 Geräten ausreichend

**Warum nicht hAP ax3 (~120 €)?**
- WiFi 6 für Wohnung irrelevant
- 1 GB RAM für reinen Router-Betrieb irrelevant
- EU-Sendeleistungsbeschränkung gilt für alle Router gleichermaßen — kein echter Vorteil
- Vorhandener WLAN-Repeater als Backup für entfernte Räume (Bett ~10m, 2 Wände)

**Warum nicht hAP ac3 (~90 €)?**
- Schlechteres Preis-Leistungs-Verhältnis als ac2 ohne relevanten Mehrwert

### Modem: Technicolor TC4400-EU (gebraucht, ~50–100 €)

- Einziges bewährte reine DOCSIS 3.1 Modem für Pyur (Community-bestätigt)
- Läuft im Bridge-Modus — Router übernimmt alles
- Nur auf eBay/Kleinanzeigen verfügbar

**Alternative:** Fritz!Box Cable (gebraucht, ~40–60 €) im Bridge-Modus — offiziell von Pyur gelistet, einfacher zu finden.

**Vor Kauf prüfen:** Pyur anrufen und fragen ob ein Modem automatisch mitgeliefert wird — manche Kabelanbieter schicken es ohne separate Buchungsoption.

---

## Gesamtkosten

| Komponente | Preis |
|---|---|
| MikroTik hAP ac2 (gebraucht) | ~40–60 € |
| Technicolor TC4400-EU (gebraucht) | ~50–100 € |
| Pyur Anschluss (einmalig) | 19,99 € |
| **Gesamt Hardware** | **~90–160 € einmalig** |
| **Laufend** | **28–30 €/Monat** |

---

## Netzwerk-Setup

```
Koaxialdose (Zimmer)
  → TC4400-EU / Fritz!Box Cable (Bridge-Modus)
  → MikroTik hAP ac2 (Router + WLAN)
      → OptiPlex 3090 (per LAN-Kabel)
      → weitere Geräte
      → WLAN-Repeater (falls Bett/andere Räume nötig)
```

---

## Jarvis REST API

RouterOS 7 REST API: `https://<router-ip>/rest/<resource>`

Authentifizierung per HTTP Basic Auth, JSON-Responses. Beispiele:
- Verbundene Geräte: `GET /rest/ip/arp`
- DHCP-Leases: `GET /rest/ip/dhcp-server/lease`
- Firewall-Regel erstellen: `POST /rest/ip/firewall/filter`
- Interface-Status: `GET /rest/interface`

API-Keys pro Benutzer konfigurierbar. Aktivierung: `www`- oder `www-ssl`-Dienst in RouterOS aktivieren.

---

## Routerfreiheit Deutschland

Provider muss auf Anfrage herausgeben:
- PPPoE-Benutzername + Passwort (oder DHCP/IPoE)
- VLAN-ID (bei Pyur prüfen)
- SIP-Daten falls Festnetz genutzt

Beim ersten Verbinden erscheint Pyur-Vorschaltseite → MAC-Adresse wird automatisch registriert. Falls das fehlschlägt: Pyur-Support anrufen und CM-MAC manuell eintragen lassen.

---

## Offene Punkte

- [ ] Pyur anrufen: Wird Modem mitgeliefert?
- [ ] Cashback-Deal prüfen: Konditionen und Auszahlungszeitpunkt
- [ ] 250 oder 500 Mbit/s Tarif wählen (Empfehlung: 500 für 2 €/Monat Aufpreis)
- [ ] TC4400-EU auf Kleinanzeigen/eBay suchen, alternativ Fritz!Box Cable
- [ ] RouterOS 7 auf ac2 prüfen beim Kauf
