# Hardware — Optionen, Preise, Empfehlung

Stand: April 2026. Alle Preise in EUR, gerundet, für DE-Versand. Vor Bestellung tagesaktuell prüfen.

## TL;DR — Zwei empfohlene Setups

| | **Sparvariante (5V/Li-Ion)** | **Robustvariante (12V/LiFePO4)** |
|---|---|---|
| Board | FireBeetle 2 ESP32-E | FireBeetle 2 ESP32-E |
| Sensoren | 4× kapaz. v1.2 + selbst lackieren | 4× DFRobot SEN0308 IP65 |
| Pumpen | 4× 5V Tauchpumpe (Tank erhöht) | 4× R385 12V Membranpumpe |
| Schaltung | 4-Kanal-Relais oder MOSFETs | 4× IRLZ44N + Flyback |
| Solar | 6V/6W IP65 | 12V/10W mit Halterung |
| Akku | 2× 18650 + Schutz + CN3791 | LiFePO4 12V 6Ah Eremit + PWM-Regler |
| Tank | 65L Mörtelkübel + Selbstbau-Deckel | 30L Wasserkanister lebensmittelecht |
| Gehäuse | Berrybase IP66 125×125×75 | BOPLA IP65 200er-Klasse |
| **Gesamt** | **~150–180 EUR** | **~270–310 EUR** |

→ Empfehlung als Einstieg: **Sparvariante**, dann iterieren. Falls 12V-Pumpen sicherer wirken, gleich Robustvariante (R385 sind sehr gut dokumentiert und im Hobby-Bereich Standard).

---

## 1. Mikrocontroller

| Board | Deep-Sleep | LiPo-Charger | Preis | Bemerkung |
|---|---|---|---|---|
| **DFRobot FireBeetle 2 ESP32-E** | **~10 µA** ohne Mods | ja, USB-C | **~15 EUR** | Empfehlung |
| FireBeetle ESP32 V1 | ~16 µA | ja | ~16 EUR | Vorgänger |
| LOLIN32 Lite | nominell 100 µA, real >1 mA ohne Mod | ja (TP4054) | ~7–9 EUR | Bastelei nötig |
| ESP32-DevKit Standard | ~150 µA | nein | ~6–8 EUR | nur mit externer Charger-Schaltung |

**Empfohlen:** FireBeetle 2 ESP32-E. Out-of-the-box 10 µA Deep Sleep, integrierter LiPo-Lader, USB-C, ESPHome-supported. Aufpreis vs. LOLIN32 wird durch wegfallende Bastelei aufgewogen.

Quellen: [Eckstein](https://eckstein-shop.de/DFRobot-FireBeetle-2-ESP32-E-IoT-Microcontroller-Supports-Wi-Fi-Bluetooth-EN), [Berrybase](https://www.berrybase.de/dfrobot-firebeetle-esp32-e-iot-microcontroller-mit-header-wlan-bluetooth), [DFRobot Wiki](https://wiki.dfrobot.com/dfr0654/docs/23260)

## 2. Bodenfeuchte-Sensoren (kapazitiv, NIE resistiv)

| Sensor | Outdoor-tauglich | Preis/Stk | Bemerkung |
|---|---|---|---|
| **DFRobot SEN0308 IP65** | ja, vergossen | **~12–15 EUR** | Empfehlung für 4-Saison-Outdoor |
| Berrybase „Analoger kapaz. Bodenfeuchtesensor" | nein, nachbearbeiten | ~3–4 EUR (5er-Pack günstiger) | Mit Plasti-Dip + Schrumpfschlauch okay |
| AZ-Delivery v1.2 | nein, nachbearbeiten | ~4–5 EUR | gleiche Liga |
| AliExpress v1.2 NoName | Lotterie | ~1–2 EUR | NE555-Klone, oft kein 3.3V-Regler — vermeiden |

**Wichtig zur v1.2:** Pad U2 prüfen — wenn nur 0R-Brücke statt 3.3V-Regler, braucht der Sensor zwingend 5V und der ADC-Wert driftet. v2.0 hat oft TLC555 + Regler, ist 3.3V-tauglich.

**Outdoor-Hardening (für günstige Sensoren):**
- PCB-Schnittkanten mit Nagellack/Plasti-Dip versiegeln
- Elektronik-Kopf mit Schrumpfschlauch + Heißkleber abdichten
- Schwarzer Plastik-Stab wird nach 1–2 Saisons UV-spröde

Quellen: [DFRobot SEN0308](https://www.dfrobot.com/product-2054.html), [Berrybase Sensor](https://www.berrybase.de/en/analogue-capacitive-soil-moisture-sensor), [AZ-Delivery v1.2](https://www.az-delivery.de/en/products/bodenfeuchte-sensor-modul-v1-2), [CavePearl-Hack](https://thecavepearlproject.org/2020/10/27/hacking-a-capacitive-soil-moisture-sensor-for-frequency-output/)

## 3. Pumpen vs. Magnetventile

### Variante A — 4 Pumpen (empfohlen)

| Pumpe | Spannung | Förderleistung | Förderhöhe | Strom | Preis |
|---|---|---|---|---|---|
| **R385 Membranpumpe** | 6–12V | ~1.5–2 L/min | ~2 m | ~0.5–0.7 A | ~7–9 EUR/Stk |
| JT-160A Membranpumpe | 12V | ~1.5–2 L/min | ~3 m | ~0.5 A | ~7–12 EUR/Stk |
| AZ-Delivery 3–6V Tauchpumpe | 3–6V (im Tank) | ~120 L/h | 1.2 m | ~220 mA | 4,99 EUR/Stk |

- R385 hält kurze Pulse (1–2 min/Tag) gut aus, Lebensdauer 500–1000 h.
- Tauchpumpen funktionieren NUR, wenn Tank höher als Kästen oder Förderhöhe (1–1.5 m) reicht. Auf Balkon mit Tank am Boden + Kästen am Geländer reicht das oft NICHT → Membranpumpe ist sicherer.

### Variante B — 1 Pumpe + 4 Magnetventile (nicht empfohlen)

- Bistabile Mini-Ventile im Hobby-Bereich kaum verfügbar (nur Industrie ~50–80 EUR/Stk).
- Monostabile 12V-Ventile (~5–8 EUR/Stk Aliexpress, ~10–15 EUR Reichelt) ziehen 250–400 mA dauerhaft während offen — bei sequentieller Bewässerung ok.
- Mehr Schlauchverlegung, Single-Point-of-Failure (eine Pumpe für alles), komplexere Drucklogik.
- Preis ähnlich wie A (~40 EUR), aber weniger robust.

### Schaltlogik für ESP32

| Option | Preis | Hinweis |
|---|---|---|
| **IRLZ44N Logic-Level-MOSFET** | ~1 EUR/Stk Reichelt | Empfohlen, schaltet sauber bei 3.3V — Pull-Down + Flyback-Diode 1N4007 nicht vergessen |
| IRF520-Modul | 1–3 EUR/Stk | **NICHT** Logic-Level, schaltet bei 3.3V nicht voll durch — vermeiden |
| 4-Kanal-Relaismodul AZ | 5,99 EUR | Einfachste Lösung, klickt aber, ~70 mA pro Relais im aktiven Zustand |

→ **Empfehlung Variante A:** 4× R385 + 4× IRLZ44N + 4× 1N4007 = **~35 EUR** (bei Aliexpress) bis ~45 EUR (Reichelt/Berrybase).

Quellen: [AZ-Delivery Wasserpumpe](https://www.az-delivery.de/products/wasserpumpe), [Berrybase Pumpen](https://www.berrybase.de/sensoren-module/aktoren/pumpen), [Reichelt IRLZ44N](https://www.reichelt.de/), [Eckstein](https://eckstein-shop.de/)

## 4. Solar + Akku + Laderegler

### Setup A — 1S Li-Ion (5V-Welt, ~35 EUR)

| Komponente | Preis | Quelle |
|---|---|---|
| 6V/6W Solarpanel IP-fest | ~18 EUR | eBay/Amazon |
| CN3791 MPPT-Laderegler 6V→1S Li-Ion 2A | ~5 EUR | [Androegg](https://www.androegg.de/shop/cn3791-mppt-solar-akku-laderegler-lithium-modul-6v-9v-12v-2a-4-2v-%C2%B1-1/) |
| 2× 18650 geschützt (mit PCB) + Halter | ~12 EUR | Berrybase |
| **Summe** | **~35 EUR** | |

Vorteil: kompakt, billig, ESPHome läuft via 3.3V-LDO am ESP32-Board. Pumpen mit 5V-Tauchvariante direkt versorgbar oder via Step-Up auf 12V für R385.

### Setup B — 12V LiFePO4 (~85 EUR, robuster)

| Komponente | Preis | Quelle |
|---|---|---|
| 12V/10W Solarpanel + Halterung | ~25 EUR | Solarkontor, Aliexpress |
| PWM-Regler 12V/10A (Offgridtec) | ~18 EUR | [Offgridtec](https://www.offgridtec.com/offgridtec-pwm-pro-laderegler-12v-24v-10a-usb.html) |
| LiFePO4 12V 6Ah mit BMS | ~43 EUR | [Eremit](https://www.eremit.de/p/12v-6ah-lifepo4-flach-mit-bms) |
| **Summe** | **~86 EUR** | |

Vorteil: 72 Wh Reserve (statt 18 Wh bei A), 12V-Pumpen direkt versorgbar (kein Step-Up), BMS integriert, 3000+ Zyklen, langlebiger.
Nachteil: schwerer, teurer.

### Schutzschaltung — Pflichten

- **1S Li-Ion:** Tiefentladeschutz Pflicht (TP4056 schützt nur beim Laden!) → entweder geschützte 18650 mit PCB oder externes Mini-BMS HX-1S (~1 EUR).
- **LiFePO4 fertig konfektioniert:** BMS bereits eingebaut.
- **AGM:** Solarregler übernimmt Last-Cutoff bei ~11.5V.

### Frostverhalten / Winterbetrieb

- **LiFePO4 darf unter 0°C NICHT geladen werden** — modernes BMS sperrt das.
- Li-Ion 18650: Entladen bis −20°C okay, Laden <0°C zerstört Zelle.
- AGM lädt langsam auch bei Frost.
- **Pragmatischer Ansatz:** System Nov–Feb einmotten, Akku in die Wohnung. Pflanzen brauchen im Winter eh kaum Wasser.

Quellen: [Eremit LiFePO4](https://www.eremit.de/p/12v-6ah-lifepo4-flach-mit-bms), [Androegg CN3791](https://www.androegg.de/shop/cn3791-mppt-solar-akku-laderegler-lithium-modul-6v-9v-12v-2a-4-2v-%C2%B1-1/), [Offgridtec PWM](https://www.offgridtec.com/offgridtec-pwm-pro-laderegler-12v-24v-10a-usb.html)

## 5. Tank + Schläuche + Tropfer

### Tank-Größe

Bedarfsrechnung: 4 Kästen × 500 mL/Tag × 7 Tage = **14 L/Woche**. Mit Reserve **20–30 L** sinnvoll. Volle 30 L = ~30 kg → Balkon-tauglich, aber Boden tragfähig prüfen.

| Option | Volumen | Preis | Bemerkung |
|---|---|---|---|
| **Wasserkanister 30 L mit Auslaufhahn (lebensmittelecht)** | 30 L | ~30 EUR | Empfehlung — geschlossen, keine Algen/Mücken |
| Autopot Wassertank 30L (Hornbach) | 30 L | ~30–40 EUR | Speziell für Bewässerung |
| Mörtelkübel PE 65L eckig (Hornbach) | 65 L | ~10–15 EUR | Sparvariante, Selbstbau-Deckel nötig |
| Camping-Kanister 10 L | 10 L | ~8 EUR | Zu klein, 2× pro Woche füllen |

### Schläuche & Verbinder

- **6 mm Silikonschlauch** ist Standard (4 mm nur für Endstrang/Tropfer).
- Cellpack 6 mm Meterware (Hornbach): ~1–2 EUR/m, alternativ Aquaristik 4/6 mm: ~1,30 EUR/m.
- Blumat T-Stück 8×3×8: 0,99 EUR/Stk.
- **Regulierbare Tropfer** (z.B. GARDENA 1–8 L/h, 5er): 7,99 EUR — sinnvoll für gleichmäßige Verteilung.

→ **Setup:** 5 m Schlauch + 4 T-Stücke + 4 Tropfer = **~15 EUR**.

### Wasserstandssensor

| Sensor | Preis | Bemerkung |
|---|---|---|
| **Float-Switch** (SCHABUS / H-TRONIC) | 3–8 EUR | Empfehlung MVP — leer/voll reicht |
| JSN-SR04T (wasserdichter Ultraschall) | 5–10 EUR | v2 für Pegel-% |
| HC-SR04 (nicht wasserdicht) | ~2 EUR | nur in geschlossenem Tank von oben |
| Kapazitiv außen am Tank | 10–20 EUR | berührungsfrei |

Quellen: [Hornbach 30L Kanister](https://www.hornbach.ch/de/p/wasserkanister-mit-auslaufhahn-lebensmittelecht-30-l/10289577/), [Hornbach Silikonschlauch](https://www.hornbach.de/p/cellpack-silikonschlauch-transparent-6-mm-meterware/8090413/), [Hornbach Mörtelkübel](https://www.hornbach.de/p/moertelkuebel-pe-eckig-typ-65-72-5-cm-x-38-5-cm-x-29-cm/274282/), [Reichelt Float Switch](https://www.reichelt.com/de/en/shop/product/mini_float_switch_plastic-243394)

## 6. Outdoor-Gehäuse & Kleinteile

| Posten | Preis | Quelle |
|---|---|---|
| Berrybase IP66 125×125×75 mm | ~8–10 EUR | [Berrybase](https://www.berrybase.de/wasserfeste-plastik-gehaeuse-ip66-125mm-x-125mm-x-75mm) |
| BOPLA IP65 150×150×57 mm (Reichelt) | ~15–20 EUR | [Reichelt](https://www.reichelt.at/at/de/shop/produkt/industriegehaeuse_150x150x57_mm_ip65-344843) |
| BOPLA IP65 300×160×120 mm | ~25–35 EUR | [Reichelt](https://www.reichelt.at/at/de/shop/produkt/industriegehaeuse_300x160x120_mm_ip65-344898) |
| Kabelverschraubungen M12/16/20/25 (40er-Set) | ~10–15 EUR | [Arli24](https://www.arli24.de/industrieheimwerker/kabelverschraubungen/set/1807/arli-kabelverschraubung-ip68-schwarz-m12-m16-m20-m25-40er-set) |
| Schlauchschellen 6 mm (10er) | ~3 EUR | Aliexpress |
| Outdoor-Tape (3M VHB) | ~5 EUR | Hornbach |
| UV-stabile Kabelbinder schwarz | ~3 EUR | beliebig |
| Edelstahlschrauben/Distanzbolzen | ~5 EUR | Hornbach |

**Subsumme Gehäuse + Kleinteile: ~25–40 EUR**

## Stückliste Komplett

### Sparvariante (5V/Li-Ion, ~150–180 EUR)

| Posten | EUR |
|---|---|
| FireBeetle 2 ESP32-E | 15 |
| 4× Bodenfeuchte v1.2 + Plasti-Dip | 16 |
| 4× R385 + Step-Up auf 12V (oder 4× Tauchpumpe wenn Tank erhöht) | 30 |
| 4× IRLZ44N + 1N4007 | 6 |
| 6V/6W Solar + CN3791 + 2× 18650 geschützt | 35 |
| Mörtelkübel 65L + Selbstbau-Deckel | 12 |
| Schlauch + T-Stücke + Tropfer | 15 |
| Float-Switch | 5 |
| IP66-Gehäuse + Verschraubungen | 15 |
| Kleinteile | 15 |
| **Summe** | **~165 EUR** |

### Robustvariante (12V/LiFePO4, ~270–310 EUR)

| Posten | EUR |
|---|---|
| FireBeetle 2 ESP32-E | 15 |
| 4× DFRobot SEN0308 IP65 | 55 |
| 4× R385 12V Membranpumpe | 30 |
| 4× IRLZ44N + 1N4007 | 6 |
| 12V/10W Solar + PWM-Regler 10A + LiFePO4 12V 6Ah | 86 |
| 30L Wasserkanister lebensmittelecht | 30 |
| Schlauch + T-Stücke + Tropfer | 15 |
| Float-Switch | 5 |
| BOPLA IP65 + Verschraubungen | 30 |
| Kleinteile | 15 |
| **Summe** | **~287 EUR** |

## Offene Fragen / Entscheidungen vor dem Kauf

1. **Welche Pflanzen?** → bestimmt Wasserbedarf und ggf. Tankgröße.
2. **Tank am Boden oder erhöht stellbar?** → entscheidet Membranpumpe vs. Tauchpumpe.
3. **Sensoren billig + selbst lackieren oder gleich IP65 (DFRobot)?** Differenz ~40 EUR.
4. **5V- oder 12V-Welt?** Treibt die Architektur (Step-Up vs. direkt).
5. **Status auf dem Handy nötig?** WLAN auf dem Balkon erreichbar? → sonst LED am Gehäuse + ESPHome via lokalem Hotspot.
6. **Float-Switch reicht oder gleich Pegel-%?** Differenz ~5 EUR.
