# Bauteile-Liste — Companion-Build

> Zubehör für den Jarvis-Companion-Build auf Basis der **Waveshare ESP32-S3-LCD-1.28** (37 mm rund, 1.28″ GC9A01-Display, IMU on-board, USB-C, LiPo-Charger).
> Ziel-Funktionsumfang: Tamagotchi-Pet + Jarvis-PTT-Voice + Universal-Remote (IR) + Schlaf-/Health-Tracking (ersetzt Mi Band).
> Stand: 2026-04-29

## A — Stromversorgung

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| LiPo-Akku 300 mAh | 503040 (5×30×40 mm), JST MX1.25 | ~6 € | Schlaf-Tracking-tauglich (~2–3 Tage Laufzeit) |
| LiPo-Akku 500 mAh | 603048 (6×30×48 mm) | ~7 € | Mehr Reserve, etwas dicker |
| LiPo-Akku 120 mAh | 501225 | ~4 € | Falls Mini-Bauform — kein Sleep-Mode realistisch |
| Steckverbinder MX1.25 mit Kabel | 2-pin pigtail | ~2 € | Falls Akku kein passendes Kabel hat |
| POGO-Pin Magnet-Charger (4-Pol) | 2.5 × 4 mm | ~5 € | Optional: kein USB-C-Stöpseln mehr, magnetisches Aufladen |

**Wichtig:** Polarität des LiPo-Anschlusses am Board prüfen, bevor anstecken — falscher Pol = totes Board.

## B — Audio (für Jarvis-Voice)

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| INMP441 I2S MEMS-Mikro | 14×4 mm | ~3 € | Voice-Capture, sauberes Signal |
| SPH0645 I2S MEMS-Mikro | Alternative zu INMP441 | ~4 € | Manche bevorzugen das wegen besserer Bässe |
| MAX98357A I2S-Verstärker | 18×14 mm | ~3 € | Speaker-Treiber |
| Mini-Speaker 8 Ω 1 W | 13 mm rund oder 11×15 mm | ~2 € | Antwort-Wiedergabe |
| Speaker 8 Ω 0.5 W | 8 mm rund | ~2 € | Falls Platz extrem knapp |
| Piezo-Buzzer | 12 mm | ~1 € | Nur Beeps, kein Speech — viel kleiner |

## C — Health-Sensoren

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| MAX30102 PPG-Modul | 14×14 mm, I2C | ~5 € | Puls + SpO2 + HRV → Schlaftracking + Spot-Check |
| MAX30105 | etwas neuer | ~6 € | Quasi gleich, gelegentlich besser verfügbar |
| SHT41 Temp+Humid | Breakout 10×10 mm | ~5 € | Raumklima → "zu trocken hier"-Habits |
| AHT20 Temp+Humid | 3×3 mm | ~3 € | Günstige SHT41-Alternative |
| APDS-9960 | 16×16 mm | ~5 € | Gesten überm Display + Lichtsensor → Auto-Helligkeit |
| MLX90614 IR-Thermometer | 5×5 mm | ~12 € | Berührungslose Temperatur (Stirn, Tasse, Pet-Spielerei) |

**Vorteil:** Alle I2C — teilen sich SDA + SCL → nur 2 GPIOs gesamt für die ganze Sensor-Kollektion.

## D — Universal Remote

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| IR-LED 940 nm | 5 mm Standard | ~0.30 € | Sender für TV/HiFi/Klima |
| 2N2222 NPN-Transistor | TO-92 | ~0.20 € | LED-Treiber (mehr Strom = mehr Reichweite) |
| 220 Ω Widerstand | 0.25 W | ~0.05 € | Strombegrenzung Basis |
| 100 Ω Widerstand | 0.25 W | ~0.05 € | LED-Strombegrenzung |
| TSOP38238 IR-Receiver | 5×6 mm | ~1 € | Optional: vorhandene Fernbedienung "anlernen" |
| nRF24L01+ Modul | 15×30 mm | ~3 € | Optional: 2.4-GHz-Funk → einige Smart-Home-Geräte |
| RFM69 433 MHz | 16×16 mm | ~5 € | Optional: alte Funksteckdosen / Garagentore |

## E — User Input + Feedback

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| Mikro-Taster 6×6×4.3 mm | THT, tactile | ~0.20 €/Stk | Feed / Play / PTT — kauf 10 Stück |
| Mikro-Taster 4×4 mm | SMD, kleiner | ~0.30 € | Falls Platz knapp |
| Side-Push-Schalter | für Crown-Position | ~1 € | Smartwatch-Look, nur Drücken (kein Drehen) |
| Tactile-Caps in Farben | für die Buttons | ~3 € Set | Bessere Haptik, optisch nett |
| WS2812B RGB-LED | 5×5 mm einzeln | ~0.50 € | Stimmungs-Farbe (subtiles "atmen") |
| Coin-Vibrationsmotor | 8×3 mm | ~2 € | Stilles Notification + haptisches "Hallo" |
| 1N4148 Schutzdiode | für Vibrationsmotor | ~0.05 € | Schutz gegen Spannungspeaks |
| 2N2222 für Vibration | s. o. | ~0.20 € | Treiber-Transistor |

## F — Gehäuse + Tragesystem

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| 3D-Druck Watch-Case mit Lugs | PLA oder Resin | 5–15 € (Druckservice) | [Raspberry Pi Case](https://www.printables.com/model/450477) oder [Watch Case mit Akku-Slot](https://www.printables.com/model/484236) |
| Silikon-Armband 20 mm | Standard-Smartwatch-Größe | ~5 € | Quick-Release, jedes Apple-Watch-Drittanbieter-Band passt |
| Federstege (Spring Bars) 20 mm | Edelstahl | ~2 €/Paar | Halten das Armband am Case |
| Schlüsselring 25 mm | Edelstahl | ~1 € | Falls *doch* Keychain-Modus parallel |
| Neodym-Magnet 20×1 mm oder 5×1 mm | rund | ~1 € | Für Magnet-Case-Variante |
| 0.3 mm Polycarbonat-Folie | Display-Schutz | ~3 € | Glas wäre ideal, aber schwer beschaffbar |
| O-Ring Set | für IP54 | ~5 € | Schweiß-Schutz beim Tragen |

## G — Verkabelung + Mechanik

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| Silikon-Litze 30 AWG (Set 6 Farben) | je 5 m | ~8 € | Hauptverkabelung — flexibel, hitzefest |
| Schrumpfschlauch-Sortiment | 1–6 mm | ~5 € | Saubere Lötstellen |
| Female 2.54-mm-Header (Sockel) | 1×40 Pin | ~2 € | Falls Sensoren steckbar statt verlötet |
| Male Header (Stiftleiste) | 1×40 Pin | ~1 € | Komplement |
| Kapton-Tape | 5 mm breit | ~3 € | Isolation in engen Cases |
| Doppelseitiges Klebeband 3M | 0.5 mm dünn | ~5 € | Sensoren im Case fixieren |
| 0.5-mm-Kupferdraht (lackiert) | 5 m | ~3 € | Extrem enge Verbindungen direkt auf der Platine |

## H — Werkzeug (überspringen falls vorhanden)

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| Lötkolben | Pinecil V2 oder TS101 | ~70 € | Klein, präzise, USB-C — viel besser als Stations-China-Ware |
| Lötzinn 0.5 mm | bleifrei mit Flussmittel | ~12 € | Dünn ist Pflicht für SMD/MEMS |
| Flussmittel-Paste | Amtech NC-559 oder ähnlich | ~10 € | Macht jede Lötstelle sauber |
| Entlötlitze 2 mm | Goot Wick | ~5 € | Fehler korrigieren |
| Pinzetten-Set ESD | gebogen + gerade | ~10 € | SMD-Handling |
| Helping Hands / PCB-Holder | mit Lupe | ~15 € | Drittes Paar Hände |
| Multimeter | beliebig | ~20 € | Polung Akku, Durchgangsprüfer |
| Logic Analyzer 8-Kanal | USB | ~10 € (China) | Optional: I2C/I2S debuggen |
| Heißluftstation | für SMD | ~50 € | Optional: nur für SMD-Umlöten |
| Drahtschneider/-zange | klein | ~10 € | Klassisch |
| Abisolierzange | für 30 AWG | ~15 € | Wichtig — normale Zangen schaffen 30 AWG schlecht |

## I — Programmierung + Computer-Anbindung

| Teil | Spec | Preis | Wofür / Notiz |
|------|------|-------|---------------|
| USB-C-Datenkabel | nicht nur Lade! | ~5 € | Viele Lade-Kabel haben keine Datenleitungen — wichtig! |
| USB-C-zu-USB-C 90°-Adapter | Winkel-Stecker | ~5 € | Für Test-Setup auf dem Schreibtisch |

---

## Empfehlung — Pflicht / Nice-to-have / Werkzeug

### Pflicht für den Companion-Voll-Build (Tamagotchi + Voice + Sleep-Tracking + Remote)

- LiPo 300 mAh (503040)
- INMP441 Mikro
- MAX98357A + 8-Ω-Speaker (1 W, 13 mm)
- MAX30102 (Puls/SpO2/Sleep)
- 3× Mikro-Taster 6×6 + 1× Side-Push für PTT
- WS2812B RGB-LED
- Coin-Vibrationsmotor + 2N2222 + 1N4148-Diode
- IR-LED 940 nm + 2N2222 + 100 Ω + 220 Ω
- Watch-Case-Print + 20-mm-Silikon-Armband + Federstege
- Silikon-Litze 30 AWG + Schrumpfschlauch
- USB-C-Datenkabel

**Summe ca. 60–70 €** (plus Board ~20 € + Druck ~10 €)

### Nice-to-have — später nachrüsten

- SHT41 (Raumklima) — wenn Habit-Quellen erweitert werden
- APDS-9960 (Gesten) — Spielerei
- TSOP38238 (IR-Lernmodus) — wenn fertige Fernbedienungen einlesen
- POGO-Pin Magnet-Charger — wenn USB-C-Stöpseln nervt
- O-Ring Set + Polycarbonat-Folie — für tägliches Tragen / Schweiß
- MLX90614 IR-Thermometer — Spielerei

### Werkzeug — nur kaufen, falls nicht vorhanden

Lötkolben + dünnes Lötzinn + Flussmittel + Pinzetten + Helping Hands + Multimeter sind das absolute Minimum. Ohne alles davon: extra ~120 € einplanen. Falls schon vorhanden → 0 €.

## Bezugsquellen für Deutschland

| Shop | Wofür gut |
|------|-----------|
| **Berrybase.de** | Boards, Sensor-Breakouts, Akkus — meist auf Lager |
| **Eckstein-shop.de** | Adafruit-Distribution + ESP32-Welt |
| **Reichelt.de** | Widerstände, Transistoren, Lötzubehör, Werkzeug |
| **AliExpress** | Alles billiger, 2–4 Wochen Lieferzeit |
| **Amazon.de** | Mikro-Taster, Litze, Schrumpfschlauch im Sortiment |
| **3D-Druck-Service** (Craftcloud / 3D-Druckexpress) | Case |

## Kosten-Übersicht

| Bereich | Pflicht-Build | + Nice-to-have | + Werkzeug von Null |
|---------|---------------|----------------|---------------------|
| Board (Waveshare ESP32-S3-LCD-1.28) | ~20 € | | |
| Bauteile Pflicht | ~60–70 € | | |
| 3D-Druck Case | ~10 € | | |
| Nice-to-have Add-Ons | | +20–30 € | |
| Werkzeug (falls nichts vorhanden) | | | +120 € |
| **Total** | **~90–100 €** | **~110–130 €** | **~230–250 €** |
