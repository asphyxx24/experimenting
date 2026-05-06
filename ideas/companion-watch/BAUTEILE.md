# Bauteile-Liste — Companion-Watch

> Zubehör für den Companion-Watch-Build auf Basis der **Waveshare ESP32-S3-Touch-LCD-1.28** (37 mm rund, 1.28" GC9A01-Display, kapazitiver Touch, IMU on-board, USB-C, LiPo-Charger).
> Ziel-Funktionsumfang: Jarvis-PTT-Voice + Universal-Remote (IR) + Schlaf-/Health-Tracking (ersetzt Mi Band).
> Stand: 2026-05-06

---

## A — Stromversorgung

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **LiPo-Akku 500 mAh** | [EREMIT 403040](https://www.eremit.de/p/eremit-3-7v-500mah-lipo-akku-403040) (4×30×40 mm), **mit Schutzschaltung** | 4,20 € | [eremit.de](https://www.eremit.de) | DE-Shop, 2–5 Tage Lieferung. Bei Bestellung **JST GH 1.25 mm** als Stecker anfragen — passt dann direkt ans Board |
| POGO-Pin Magnet-Charger (4-Pol) | 2.5 x 4 mm | ~5 € | AliExpress | Optional: kein USB-C-Stöpseln mehr |

**Wichtig:** Polarität des LiPo-Anschlusses am Board prüfen, bevor anstecken — falscher Pol = totes Board.

**Stecker:** EREMIT bietet Konfektionierung aller gängigen Stecker-Typen an. Bei der Bestellung im Kommentar oder per Mail angeben: *„Bitte mit JST GH 1.25 mm 2-Pin Stecker konfektionieren"*. Dann passt der Akku direkt in die Buchse des Waveshare-Boards — kein Umlöten nötig.

**Upgrade-Option:** Für längere Laufzeit später einen größeren Akku (z. B. EREMIT 903040, 800 mAh) einbauen — gleiche Fläche, nur dicker (9 mm statt 4 mm), Case-Anpassung nötig.

---

## B — Audio (für Jarvis-Voice)

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **INMP441 I2S MEMS-Mikro** | 14x4 mm | ~3 € | Berrybase / AliExpress | Voice-Capture, sauberes Signal |
| **MAX98357A I2S-Verstärker** | 18x14 mm | ~3 € | Berrybase / AliExpress | Speaker-Treiber |
| **Mini-Speaker 8 Ohm 1 W** | 13 mm rund | ~2 € | AliExpress | Antwort-Wiedergabe |

---

## C — Health-Sensoren

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **MAX30102 PPG-Modul** | 14x14 mm, I2C | ~5 € | Berrybase / AliExpress | Puls + SpO2 + HRV -> Schlaftracking |
| SHT41 Temp+Humid | Breakout 10x10 mm | ~5 € | Berrybase | Nice-to-have, später nachrüsten |

**Schlaftracking:** IMU bereits on-board (QMI8658) + MAX30102 = vollständiges Sleep-Tracking, kein extra Hardware nötig.
**I2C-Bus:** MAX30102 und SHT41 teilen sich GPIO 6/7 mit dem on-board IMU — keine zusätzlichen Pins verbraucht.

---

## D — Universal Remote

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **IR-LED 940 nm** | 5 mm Standard | ~0.30 € | Reichelt | Sender für TV/HiFi/Klima |
| **2N2222 NPN-Transistor** | TO-92 | ~0.20 € | Reichelt | LED-Treiber (mehr Reichweite) |
| **220 Ohm Widerstand** | 0.25 W | ~0.05 € | Reichelt | Strombegrenzung Basis |
| **100 Ohm Widerstand** | 0.25 W | ~0.05 € | Reichelt | LED-Strombegrenzung |
| **TSOP38238 IR-Receiver** | 5x6 mm | ~1 € | Reichelt / AliExpress | Fernbedienungen "anlernen" |

---

## E — User Input + Feedback

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **Mikro-Taster 6x6x4.3 mm** | THT, tactile | ~0.20 €/Stk | Reichelt | Feed / Play / PTT — 10 Stück kaufen |
| **Side-Push-Schalter** | für Crown-Position | ~1 € | AliExpress | PTT-Knopf, Smartwatch-Look |
| **WS2812B RGB-LED** | 5x5 mm einzeln | ~0.50 € | AliExpress | Stimmungs-Farbe ("atmen") |
| **Coin-Vibrationsmotor** | 8x3 mm | ~2 € | AliExpress | Stille Notifications |
| **1N4148 Schutzdiode** | für Vibrationsmotor | ~0.05 € | Reichelt | Schutz gegen Spannungspeaks |
| **2N2222 für Vibration** | TO-92 | ~0.20 € | Reichelt | Treiber-Transistor |
| Tactile-Caps in Farben | Set | ~3 € | AliExpress | Bessere Haptik, optional |

---

## F — Gehäuse + Tragesystem

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **3D-Druck Watch-Case mit Lugs** | PLA oder Resin | 5–15 € | Craftcloud / 3D-Druckexpress | [Watch Case mit Akku-Slot](https://www.printables.com/model/484236) |
| **Silikon-Armband 20 mm** | Standard-Smartwatch-Größe | ~5 € | AliExpress | Quick-Release |
| **Federstege 20 mm** | Edelstahl | ~2 €/Paar | AliExpress | Halten das Armband am Case |
| 0.3 mm Polycarbonat-Folie | Display-Schutz | ~3 € | AliExpress | Nice-to-have |
| O-Ring Set | für IP54 | ~5 € | AliExpress | Nice-to-have, Schweiß-Schutz |
| POGO-Pin Magnet-Charger | 4-Pol, 2.5x4 mm | ~5 € | AliExpress | Nice-to-have |

---

## G — Verkabelung + Mechanik

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **Silikon-Litze 30 AWG** | Set 6 Farben, je 5 m | ~8 € | AliExpress / Amazon | Hauptverkabelung |
| **Schrumpfschlauch-Sortiment** | 1–6 mm | ~5 € | Amazon | Saubere Lötstellen |
| **Kapton-Tape** | 5 mm breit | ~3 € | AliExpress | Isolation in engen Cases |
| **Doppelseitiges Klebeband 3M** | 0.5 mm dünn | ~5 € | Amazon | Sensoren im Case fixieren |

---

## H — Werkzeug (überspringen falls vorhanden)

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **Lötkolben** | Pinecil V2 oder TS101 | ~70 € | Amazon / AliExpress | USB-C, präzise Temperatur |
| **Lötzinn 0.5 mm** | bleifrei mit Flussmittel | ~12 € | Reichelt | Dünn ist Pflicht |
| **Flussmittel-Paste** | Amtech NC-559 o.ä. | ~10 € | AliExpress | Saubere Lötstellen |
| **Entlötlitze 2 mm** | Goot Wick | ~5 € | Reichelt | Fehler korrigieren |
| **Pinzetten-Set ESD** | gebogen + gerade | ~10 € | AliExpress | SMD-Handling |
| **Helping Hands / PCB-Holder** | mit Lupe | ~15 € | AliExpress | |
| **Multimeter** | beliebig | ~20 € | Baumarkt | Akku-Polarität prüfen — wichtig! |
| Abisolierzange | für 30 AWG | ~15 € | Amazon | Normale Zangen schaffen 30 AWG schlecht |

---

## I — Programmierung + Computer-Anbindung

| Teil | Spec | Preis | Bezug | Notiz |
|------|------|-------|-------|-------|
| **USB-C-Datenkabel** | nicht nur Lade! | ~5 € | Amazon | Viele Lade-Kabel haben keine Datenleitungen |

---

## Pflicht-Build — finale Auswahl

| Teil | Preis |
|------|-------|
| Waveshare ESP32-S3-**Touch**-LCD-1.28 | ~25 € |
| LiPo EREMIT 403040 500mAh (JST GH 1.25) | ~4 € |
| INMP441 Mikro | ~3 € |
| MAX98357A + 8-Ohm-Speaker 1W 13mm | ~5 € |
| MAX30102 (Puls/SpO2/Sleep) | ~5 € |
| IR-LED 940nm + TSOP38238 + 2N2222 + Widerstände | ~2 € |
| 3x Mikro-Taster 6x6 + 1x Side-Push (PTT) | ~2 € |
| WS2812B RGB-LED | ~1 € |
| Coin-Vibrationsmotor + 2N2222 + 1N4148 | ~3 € |
| Watch-Case-Print + 20mm-Armband + Federstege | ~12 € |
| Silikon-Litze 30 AWG + Schrumpfschlauch + Kapton | ~10 € |
| USB-C-Datenkabel | ~5 € |
| **Summe Bauteile** | **~77 €** |

### Nice-to-have — später nachrüsten

- SHT41 (Raumklima)
- O-Ring Set + Polycarbonat-Folie (tägliches Tragen / Schweiß)
- POGO-Pin Magnet-Charger (kein USB-C-Stöpseln mehr)

### Bewusst weggelassen

- MAX30105 → MAX30102 reicht, mehr Community-Support
- SPH0645 → nur Ersatz falls INMP441 nicht lieferbar
- Piezo-Buzzer → Speaker ist drin, Buzzer überflüssig
- APDS-9960 → Gesten unzuverlässig auf kleinen Boards
- MLX90614 → kein echtes Use-Case, teuer
- nRF24L01+ / RFM69 → zu nischig

---

## Bezugsquellen

| Shop | Wofür |
|------|-------|
| **[eremit.de](https://www.eremit.de)** | Akku EREMIT 403040 (JST GH 1.25 anfragen) |
| **eBay / AliExpress** | Watch-Case, Armband, Kleinteile |
| **Berrybase.de** | Board, MAX30102, INMP441, MAX98357A — schnelle Lieferung |
| **Reichelt.de** | Widerstände, Transistoren, TSOP38238, Lötzubehör |
| **AliExpress** | Litze, Schrumpfschlauch, WS2812B, Vibrationsmotor |
| **Amazon.de** | Werkzeug, Klebeband, USB-Kabel |
| **Craftcloud / 3D-Druckexpress** | Watch-Case-Druck |

---

## Kosten-Übersicht

| Bereich | Kosten |
|---------|--------|
| Board (ESP32-S3-Touch-LCD-1.28) | ~25 € |
| Bauteile Pflicht-Build | ~52 € |
| 3D-Druck Case | ~10 € |
| **Total Pflicht** | **~87 €** |
| + Nice-to-have Add-Ons | +15–20 € |
| + Werkzeug (falls nichts vorhanden) | +120 € |
