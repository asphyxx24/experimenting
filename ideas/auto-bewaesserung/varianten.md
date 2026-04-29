# Varianten im Detail — Sparsam vs. Robust

Zwei durchgerechnete Setups für die autarke Bewässerung von 4 Balkon-Blumenkästen, jeweils mit Begründung, Aufbau, Trade-offs und Aufbauplan.

Die kompakte Tabelle aus `hardware.md` ist hier ausführlich erklärt — was wofür, warum genau das, und wo die Schmerzgrenzen liegen.

---

## Gemeinsame Architektur (für beide Varianten gleich)

```
   ┌────────────┐
   │ Solarpanel │
   └─────┬──────┘
         │
   ┌─────▼──────┐    ┌────────────┐
   │ Laderegler │───▶│   Akku     │
   └────────────┘    └─────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  ESP32      │◀── 4× Bodenfeuchte-Sensor
                    │ (Deep Sleep)│◀── Float-Switch (Tank leer?)
                    └──────┬──────┘
                           │ GPIO → MOSFET-Gate
                           │
                    ┌──────▼──────────────────────────┐
                    │ 4× IRLZ44N (Logic-Level-MOSFET) │
                    └──────┬──────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        ▼                  ▼                  ▼
   Pumpe Kasten 1    Pumpe Kasten 2    Pumpe Kasten 3/4
                           │
                    ┌──────▼──────┐
                    │ Wassertank  │
                    └─────────────┘
```

**Was passiert sequentiell:**
1. ESP32 wacht alle paar Stunden aus Deep Sleep auf (~10 µA Ruhestrom).
2. Liest die 4 ADC-Werte der kapazitiven Bodensensoren.
3. Falls ein Kasten unter Schwellwert: schaltet zugehörigen MOSFET → Pumpe läuft 5–15 Sekunden → wartet → misst neu.
4. Liest Float-Switch im Tank (leer/voll). Bei leer: LED blinkt, optional WLAN-Push.
5. Schaltet alles ab, geht in Deep Sleep.

**Warum diese gemeinsame Basis:**
- **FireBeetle 2 ESP32-E** ist der einzige günstige ESP32, der out-of-the-box <50 µA im Deep Sleep schafft. LOLIN32 Lite braucht Hardware-Mods, Standard-DevKits sind viel zu hungrig für Solar-Betrieb mit kleinem Akku.
- **R385-Membranpumpen** sind im Hobby-Bereich Standard, gut dokumentiert, selbstansaugend (Tank kann unten stehen), 1.5–2 L/min Förderleistung.
- **IRLZ44N** statt IRF520: Der IRLZ44N ist Logic-Level (schaltet bei 3.3V vom ESP32 voll durch). IRF520-Module sind zwar überall billig zu finden, aber NICHT Logic-Level — sie schalten bei 3.3V nicht sauber, das führt zu Spannungsabfällen am MOSFET, Hitze und unzuverlässigem Pumpenstart.
- **Variante A (eine Pumpe pro Kasten)** schlägt Variante B (eine Pumpe + 4 Magnetventile), weil:
  - Bistabile Mini-Magnetventile sind im Hobby-Bereich nicht verfügbar (nur Industrie ~50 EUR/Stk).
  - Monostabile Ventile brauchen Dauerstrom während offen → Akku-unfreundlich.
  - Mehr Schlauchverlegung, höhere Komplexität (Ventil + Pumpe synchron schalten).
  - Single Point of Failure: Pumpe defekt = alle Kästen vertrocknen.

---

## Variante 1 — Sparvariante (5V/Li-Ion, ~165 EUR)

**Idee:** kleinster sinnvoller Aufbau, gerade so robust für eine Saison, mit Komponenten, die man auch in anderen Bastelprojekten weiterverwenden kann.

### Stückliste

| Posten | Modell | Preis (EUR) |
|---|---|---|
| Mikrocontroller | DFRobot FireBeetle 2 ESP32-E | 15 |
| Bodensensoren | 4× kapaz. v1.2 (Berrybase) + Plasti-Dip + Schrumpfschlauch | 16 |
| Pumpen | 4× R385 Membranpumpe (12V-Variante) | 30 |
| Step-Up-Wandler | 1× MT3608 oder XL6009 (3.7V → 12V, 2A) | 3 |
| MOSFETs | 4× IRLZ44N + 4× 1N4007 Flyback-Diode + Pull-Down-R | 6 |
| Solarpanel | 6V / 6W IP-fest, monokristallin | 18 |
| Laderegler | CN3791 MPPT (6V→1S Li-Ion, 2A) | 5 |
| Akku | 2× 18650 geschützt (Samsung INR oder LG, je ~2500 mAh) parallel im Halter | 12 |
| Tank | Mörtelkübel PE 65L + Selbstbau-Deckel (Sperrholz/Plexiglas + Dichtung) | 12 |
| Schläuche | 5 m Silikon 6 mm + 4× T-Stück + 4× regulierbare Tropfer | 15 |
| Tank-Sensor | Float-Switch SCHABUS Mini | 5 |
| Gehäuse | Berrybase IP66 125×125×75 mm + 4× M16-Verschraubung | 15 |
| Kleinteile | Schellen, UV-Kabelbinder, VHB-Tape, Schrauben | 15 |
| **Summe** | | **~165** |

### Aufbau-Erläuterung

**Strom-Pfad:** Solarpanel (6V) → CN3791 → 2× 18650 parallel (3.7V nominal, ~5000 mAh) → FireBeetle ESP32 (zieht Strom direkt vom Akku via JST-PH, internen 3.3V-LDO nutzen).

Pumpen brauchen 12V. Da der Akku nur 3.7V liefert, kommt ein **Step-Up-Wandler** (MT3608 oder XL6009) vor die MOSFETs. Den Step-Up schaltet man idealerweise nicht permanent ein — entweder über einen High-Side-MOSFET oder schlicht in Kauf nehmen, dass der Wandler ~5–10 mA Ruhestrom zieht (bei 5000 mAh Akku verschmerzbar, bedeutet ~30 Tage Reserve ohne Sonne).

Alternativ: **5V-Tauchpumpen** (AZ-Delivery 3-6V, ~5 EUR/Stk) statt R385. Spart Step-Up, aber Tank muss höher als Kästen stehen oder mindestens auf gleicher Höhe — Förderhöhe nur 1–1.5 m. Bei normalem Balkon mit Tank am Boden + Kästen am Geländer reicht das oft NICHT.

**Sensor-Hardening:** Die billigen kapazitiven v1.2-Sensoren halten outdoor nicht ohne Vorbereitung. Vor dem Einbau:
1. PCB-Schnittkanten und Lötstellen mit Plasti-Dip oder klarem Nagellack zweimal beschichten.
2. Den Elektronik-Kopf (oberes Drittel mit NE555 / TLC555) mit Schrumpfschlauch + bisschen Heißkleber an der Kabeldurchführung wasserdicht machen.
3. **Wichtig:** Pad U2 prüfen — wenn da nur eine 0R-Brücke statt 3.3V-Regler ist, braucht der Sensor zwingend 5V Versorgung und der ADC-Wert driftet mit der Spannung. Bei 3.7V vom Akku wird's eng. Lieber gleich v2.0-Variante mit TLC555 + Spannungsregler kaufen oder darauf achten.

**Tank:** Der 65L-Mörtelkübel ist die Sparlösung. Vorteil: 4-fache Bedarfsdeckung, lange Reserve. Nachteil: offen → Algen, Mücken, Verdunstung. Gegenmaßnahme: Selbstbau-Deckel aus 6 mm Sperrholz oder Plexiglas mit Loch für Auffüllen + Loch für Schlauchabgang + Dichtung (selbstklebendes Moosgummi-Profil).

**Gehäuse:** Das Berrybase IP66 125×125×75 ist knapp aber machbar — ESP32, Step-Up und 4× MOSFET passen, wenn man's auf einer Lochrasterplatte ordentlich anordnet. Vier Kabeldurchführungen (M16) für Sensorkabel, Pumpenkabel, Solar-Eingang und ggf. Antenne. Gehäuse mit Verschraubungen NACH UNTEN montieren, damit kein Wasser durch die Schraubenlöcher der Wand reinläuft.

### Wann diese Variante richtig ist

- Du willst klein anfangen und das Konzept erstmal validieren.
- Eine Saison soll reichen, dann ggf. ausbauen.
- Du hast die Komponenten teilweise schon rumliegen (ESP32, MOSFETs, Akkus).
- Tank kann offen sein oder du baust einen Deckel.

### Schmerzgrenzen / wann es nicht reicht

- **Wintertauglichkeit ist gering** — Li-Ion mag Frost noch weniger als LiFePO4, Akku raus ab November.
- **Step-Up-Wandler erhöht Komplexität** und ist eine zusätzliche Fehlerquelle.
- **Billig-Sensoren sind Lotterie** — ein Sensor pro Saison könnte ausfallen.
- **Mörtelkübel sieht nicht hübsch aus** auf dem Balkon (Designaspekt).

---

## Variante 2 — Robustvariante (12V/LiFePO4, ~287 EUR)

**Idee:** durchdachtes Setup, das mehrere Saisons übersteht, weniger Bastelei, weniger Sorgen, dafür ~120 EUR teurer.

### Stückliste

| Posten | Modell | Preis (EUR) |
|---|---|---|
| Mikrocontroller | DFRobot FireBeetle 2 ESP32-E | 15 |
| Bodensensoren | 4× DFRobot SEN0308 (IP65, vergossen) | 55 |
| Pumpen | 4× R385 Membranpumpe (12V) | 30 |
| MOSFETs | 4× IRLZ44N + 4× 1N4007 Flyback-Diode + Pull-Down-R | 6 |
| Step-Down 12V→5V | MP1584 oder LM2596 (für ESP32-Versorgung) | 3 |
| Solarpanel | 12V / 10W monokristallin mit Aluminium-Rahmen + Halterung | 25 |
| Laderegler | PWM-Regler 12V/10A (Offgridtec o.ä.) | 18 |
| Akku | LiFePO4 12V 6Ah Eremit (mit BMS, IP54-Gehäuse) | 43 |
| Tank | 30L Wasserkanister mit Auslaufhahn, lebensmittelecht | 30 |
| Schläuche | 5 m Silikon 6 mm + 4× T-Stück + 4× regulierbare Tropfer | 15 |
| Tank-Sensor | Float-Switch SCHABUS Mini | 5 |
| Gehäuse | BOPLA IP65 200er-Klasse + 4× M16-Verschraubung | 30 |
| Kleinteile | Schellen, UV-Kabelbinder, VHB-Tape, Schrauben | 15 |
| **Summe** | | **~287** |

### Aufbau-Erläuterung

**Strom-Pfad:** Solarpanel (12V/18V Leerlauf) → PWM-Solarregler → LiFePO4 12V 6Ah → von dort 12V direkt an Pumpen-MOSFETs UND parallel Step-Down auf 5V für ESP32.

**Vorteil 12V-Welt:** Pumpen werden direkt versorgt, kein Step-Up. Der MP1584-Step-Down auf 5V für ESP32 ist effizient (>90 %) und einmal eingestellt sehr stabil. Insgesamt weniger Wandlungsverluste, sauberere Architektur.

**Akku-Wahl LiFePO4:**
- 72 Wh Reserve (gegen 18 Wh bei 1S Li-Ion) — überbrückt mehrere bewölkte Tage problemlos.
- BMS bereits eingebaut (Tiefentladeschutz, Überladeschutz, Zellbalancing).
- 3000+ Vollzyklen → 8–10 Jahre realistische Lebensdauer.
- Robuster bei Wärmeschwankungen als Li-Ion (für Outdoor-Einsatz wichtig).
- **Aber:** Laden unter 0°C zerstört die Zelle, modernes BMS sperrt das. Effekt: Im Winter (Nov–Feb) lädt nichts nach, Akku läuft auf Reserve. Pragmatisch: System November–Februar einmotten oder Akku reinholen.

**Sensoren DFRobot SEN0308:**
- Werkseitig vergossen, IP65, kein Nachbearbeiten nötig.
- Ca. 12–15 EUR/Stück → bei 4 Stück knapp 60 EUR statt 16 EUR bei billigen + Plasti-Dip.
- Aber: keine NE555-Lotterie, keine Korrosionsprobleme, mehrere Saisons erprobt im Hobby- und semi-professionellen Bereich.
- Lohnt sich, wenn man nicht jedes Frühjahr Sensoren tauschen will.

**Tank 30L Kanister:**
- Lebensmittelecht (HDPE), geschlossener Deckel mit Auslaufhahn.
- Keine Algen, keine Mückenlarven, keine Verdunstung in nennenswerter Menge.
- 30 L = 4-Kasten-Bedarf für 1–2 Wochen, Urlaubsreserve drin.
- Voll = 30 kg → vor dem Aufstellen kurz prüfen, ob Balkon das tragen kann (sollte bei modernen Balkonen kein Problem sein, bei Altbau-Holzbalkon vorsichtig).

**Gehäuse BOPLA IP65 200er:**
- Industrietauglich, UV-stabil, mehrere Saisons outdoor erprobt.
- Genug Platz für ESP32, Step-Down, 4× MOSFET, Lochrasterplatine, Klemmleiste.
- Teurer als Berrybase, aber langlebig.

### Wann diese Variante richtig ist

- Du willst das Setup mehrere Jahre laufen lassen ohne ständig Komponenten zu tauschen.
- Du hast Lust auf 12V-Solarsysteme (auch in anderen Projekten verwendbar — z.B. Balkon-Beleuchtung, Smart-Home-Sensoren).
- Optik ist wichtig (geschlossener Kanister sieht ordentlicher aus als Mörtelkübel).
- Du planst v2 mit WLAN-Anbindung und Statusüberwachung.

### Schmerzgrenzen

- **Investment** — wenn das Konzept doch nicht funktioniert, sind 287 EUR weg (bzw. die Komponenten lassen sich teilweise weiterverwenden, aber LiFePO4-Akku z.B. nicht so leicht).
- **Größere Bauform** — der LiFePO4-Akku ist faustgroß, das Solarpanel 30×30 cm. Auf einem winzigen Balkon ggf. zu sperrig.
- **Bestellaufwand** — Komponenten kommen aus 4–5 verschiedenen Shops (Eremit, Reichelt, Berrybase, DFRobot, Hornbach). Sammelbestellung schwierig.

---

## Direktvergleich

| Aspekt | Sparvariante | Robustvariante |
|---|---|---|
| **Anschaffung** | ~165 EUR | ~287 EUR |
| **Erwartete Lebensdauer** | 1–2 Saisons | 5–10 Jahre |
| **Bastelaufwand** | Mittel (Sensoren härten, Step-Up integrieren, Deckel für Tank bauen) | Niedrig (alles fertig outdoor-tauglich) |
| **Ausfallrisiko pro Saison** | Mittel — Sensoren / Step-Up / Akku-Schutz mögliche Schwachstellen | Niedrig — alle kritischen Teile industriegrade |
| **Wartung** | Sensoren ggf. jährlich nachlackieren oder tauschen | Jährliche Sichtkontrolle reicht |
| **Optik auf Balkon** | Mörtelkübel + offene Lochrasterplatine im Mini-Gehäuse | Sauberer Kanister + großzügig dimensioniertes Industriegehäuse |
| **Wintertauglichkeit** | Akku raus, alles abbauen | Akku raus, Pumpen entwässern, Rest kann draußen bleiben |
| **Skalierbarkeit auf 6+ Kästen** | Schwierig — Step-Up und 5000 mAh Akku werden eng | Einfach — 12V/72Wh hat reichlich Reserve |
| **Wiederverwertbarkeit der Teile** | Hoch (Standard-Bastelteile) | Mittel (LiFePO4 + Solarregler eher Spezialteile) |

---

## Entscheidungs-Heuristik

- **Du hast schon ESP32, 18650 und MOSFETs in der Schublade?** → Sparvariante, Materialeinsatz nochmal ~50 EUR weniger.
- **Du willst es einmal richtig machen und 5 Jahre Ruhe?** → Robustvariante.
- **Du bist unsicher, ob die Pflanzen das Konzept überleben?** → Sparvariante als Prototyp, später ggf. auf Robust upgraden (FireBeetle, Pumpen, Schläuche, Tank lassen sich mitnehmen — getauscht wird vor allem Akku/Solarregler/Sensoren).
- **Tank kann erhöht stehen (Hochregal, Geländer-Halterung)?** → Sparvariante mit 5V-Tauchpumpen statt R385+Step-Up — spart ~10 EUR und eine Fehlerquelle.

---

## Aufbauplan (für beide Varianten gleich, ~1 Wochenende)

**Phase 1 — Breadboard (2–3 Stunden)**
1. ESP32 mit einem Sensor auf Breadboard verkabeln.
2. ADC-Werte loggen — trockenen Sensor (in Luft) und feuchten (in Wasser) kalibrieren.
3. Eine Pumpe mit MOSFET ansteuern, kurz auf 10 s Pulse schalten.
4. Schwellwertlogik testen: Sensor unter X% → Pumpe an.

**Phase 2 — Hardware-Aufbau (4–6 Stunden)**
1. Lochrasterplatine bestücken: ESP32-Sockel, 4× MOSFET-Schaltung, Step-Up/Down, Sicherung, Klemmen.
2. Gehäuse bohren, Verschraubungen einsetzen.
3. Akku + Laderegler verkabeln, Solarpanel anschließen, Spannungen messen.
4. Pumpen vorbereiten: Schläuche zuschneiden, T-Stücke setzen, Tropfer am Ende.

**Phase 3 — Installation (2–3 Stunden)**
1. Tank aufstellen, Pumpen anschließen, Schläuche in Kästen verlegen.
2. Sensoren in Erde stecken (mind. 5 cm tief, mittig im Kasten).
3. Solarpanel ausrichten (Süden, Neigung ~30–45°).
4. Gehäuse montieren, Kabel zugentlastet durch Verschraubungen führen.

**Phase 4 — Software / ESPHome (2–4 Stunden)**
1. ESPHome-Config mit 4× ADC-Sensor + 4× Switch (Pumpe) + 1× Binary-Sensor (Float).
2. Automation: alle 3 h Wakeup, Sensoren lesen, bei Bedarf Pumpen einzeln triggern.
3. Optional: Home-Assistant-Anbindung für Status & manuelles Override.
4. Deep-Sleep konfigurieren — Wichtig für Akku-Laufzeit.

**Phase 5 — Tuning (über 1–2 Wochen)**
1. Schwellwerte pro Kasten anpassen (Pflanzenart, Erde, Sonnenseite).
2. Pump-Dauer anpassen — Faustregel: 5 s pumpen, 5 min warten, neu messen.
3. Logging analysieren, Korrekturen einbauen.

---

## Was als nächstes?

Wenn du dich für eine Variante entschieden hast, kann der nächste Schritt sein:
- Konkrete Einkaufsliste mit Direktlinks und Warenkorb-URLs erstellen.
- ESPHome-Config-Template vorbereiten.
- Schaltplan (KiCad/Fritzing) zeichnen.
- 3D-druckbare Halterungen für Solarpanel und Sensoren entwerfen.
