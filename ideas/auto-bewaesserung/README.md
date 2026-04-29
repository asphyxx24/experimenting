# auto-bewaesserung

**Status:** 🔬 Recherche

Automatische Bewässerung für 4 Blumenkästen auf dem Balkon — ohne Stromanschluss, ohne Wasseranschluss. Selbstbau mit ESP32 / Mikrocontroller.

## Constraints

- **4 Blumenkästen** auf dem Balkon.
- **Kein Stromanschluss** → Batterie + Solar.
- **Kein Wasseranschluss** → Vorratstank, manuell aufgefüllt.
- **Selbstbau okay**, gerne mit Claude Code / Conductor.

## Ziel-Funktionen (MVP)

1. Pro Kasten: Bodenfeuchte messen.
2. Bei zu trockenem Boden: definierte Wassermenge in den Kasten abgeben.
3. Tank-Füllstand grob überwachen, damit man weiß, wann nachfüllen.
4. Komplett autark — Solar lädt Akku, Akku versorgt Elektronik & Pumpe(n).
5. Optional v2: Status per WLAN/Bluetooth aufs Handy (Restwasser, letzte Gießung, Sensorwerte).

## Architektur-Skizze

```
    ┌──────────────┐
    │ Solarpanel   │
    └──────┬───────┘
           │
    ┌──────▼───────┐    ┌─────────────────────┐
    │ Laderegler   │───▶│ LiFePO4 / Li-Ion    │
    │ (MPPT/PWM)   │    │ Akku                │
    └──────────────┘    └──────┬──────────────┘
                               │
                        ┌──────▼──────┐
                        │  ESP32      │◀── Bodenfeuchte-Sensor x4
                        │ (deep sleep)│◀── Tank-Füllstand
                        └──────┬──────┘
                               │
                        ┌──────▼──────┐
                        │ MOSFET /    │
                        │ Relais      │
                        └──────┬──────┘
                               │
            ┌──────────────────┼──────────────────┐
            ▼                  ▼                  ▼
       Pumpe Kasten 1    Pumpe Kasten 2    ... (oder 1 Pumpe + 4 Magnetventile)
                               │
                        ┌──────▼──────┐
                        │  Wassertank │
                        └─────────────┘
```

## Zwei Wasser-Topologien

### Variante A: 1 Pumpe pro Kasten
- 4× kleine 5V/12V-Tauchpumpe oder Membranpumpe.
- Einfache Logik: Pumpe X an, Y Sekunden, Pumpe X aus.
- Mehr Pumpen = mehr Stromverbrauch & mehr Verschleißteile.

### Variante B: 1 Pumpe + 4 Magnetventile
- Eine zentrale Pumpe drückt Wasser in eine Ringleitung.
- Pro Kasten ein 12V-Magnetventil, das den Abzweig öffnet.
- Magnetventile brauchen aber dauerhaft Strom, solange offen → bistabile Ventile bevorzugen, oder kurze Öffnungszeiten.

### Variante C (energiesparendster Hack): Schwerkraft + Ventile
- Tank über Kastenniveau aufstellen.
- Nur Magnetventile öffnen, kein Pumpen nötig.
- Setzt voraus, dass auf dem Balkon Platz für erhöhten Tank ist.

→ Erste Tendenz: **Variante A mit kleinen Membranpumpen** ist am robustesten und mechanisch simpel. Variante C wäre die mit Abstand stromsparendste.

## Komponenten-Bill-of-Materials (Erstentwurf)

| Komponente | Beispiel | Anzahl | Hinweis |
|---|---|---|---|
| Mikrocontroller | ESP32 (z.B. LOLIN32 Lite, low-power tauglich) | 1 | Deep sleep ~10–50 µA |
| Bodenfeuchte-Sensor | **kapazitiv** (v1.2 / v2.0) | 4 | resistive verrotten — kapazitiv ist Pflicht |
| Pumpen | 5V/12V Membranpumpe oder kleine Tauchpumpe | 4 (Var. A) | typ. 200–500 mL/min |
| Treiber | MOSFET-Modul (IRLZ44N o.ä.) | 4 | 1 pro Last |
| Tank-Füllstand | Float-Switch oder kapazitiv außen am Tank | 1 | reicht „leer / nicht leer" |
| Solarpanel | 5–10 W, 6V oder 12V | 1 | Balkonausrichtung beachten |
| Laderegler | TP4056 (Li-Ion) oder MPPT für LiFePO4 | 1 | LiFePO4 ist temperatur-toleranter |
| Akku | 18650 Li-Ion 2× oder 12V LiFePO4 | 1 Pack | Outdoor-Temperatur beachten |
| Schläuche, T-Stücke, Dichtungen | Aquaristik-Bedarf | — | 6 mm Silikon ist Standard |
| Gehäuse | IP65-Box | 1 | Balkon = Regen + Sonne |

## Stromhaushalt (grobe Überschlagsrechnung)

- ESP32 deep sleep: ~30 µA.
- Aufwachen, messen, ggf. gießen: ~80 mA für ~10 s, alle 1–6 h.
- Pumpe an: ~300–500 mA für ~10–30 s pro Gießvorgang pro Kasten.
- Pro Tag grob: < 50 mAh durchschnittlich.
- 1× 18650 mit 2500 mAh hält rechnerisch tagelang ohne Sonne.
- Solarpanel mit 5 W liefert bei mäßiger Sonne locker das Mehrfache des Tagesbedarfs.

→ **Fazit:** Strom ist nicht der Engpass. Engpass ist Wassertank-Größe und Sensorqualität.

## Sensorlogik

- Kapazitive Bodensensoren liefern analoge Spannung → ADC am ESP32.
- Pro Sensor **kalibrieren** (Wert in trockener Erde vs. nasser Erde merken).
- Hysterese verwenden: erst gießen, wenn unter z.B. 30 % „Feuchte"; aufhören bei z.B. 70 %.
- **Nicht** kontinuierlich gießen, sondern Pulse: 5 s gießen → 5 min warten → neu messen. Verhindert Überwässerung, weil Wasser Zeit braucht, sich zu verteilen.

## Offene Fragen

- [ ] Wie groß sollte der Tank sein, damit man nur 1× pro Woche nachfüllen muss? (abhängig von Pflanzenart / Wetter)
- [ ] Kapazitive Sensoren v1.2 vs. v2.0 — welche Variante ist langlebiger im Outdoor-Dauereinsatz?
- [ ] Schlauchführung: zentral verteilt oder pro Kasten separat?
- [ ] Welches Gehäuse für Elektronik auf dem Balkon (Sonne/Regen/Frost im Winter)?
- [ ] Winterbetrieb: System einmotten oder frostfest auslegen? (LiFePO4 mag keine Ladung unter 0 °C)
- [ ] Nachfüll-Erinnerung: einfacher Float-Switch oder lieber gleich mit kapazitivem Pegelmesser?
- [ ] Status auf dem Handy nötig oder reicht eine LED am Gehäuse?

## Nächste Schritte

1. **Pflanzenarten in den Kästen klären** → bestimmt Wasserbedarf, Tank-Dimension.
2. **Variante festlegen** (A / B / C) — Tendenz: A.
3. **Stückliste konkretisieren** und einkaufen.
4. **Breadboard-Prototyp** mit 1 Kasten + 1 Sensor + 1 Pumpe.
5. **Kalibrierung & Logik** in Firmware (ESP32, Arduino-Framework oder ESPHome).
6. **Outdoor-Härtung** (Gehäuse, Schlauchführung, Solarmontage).
7. Auf 4 Kästen skalieren.

## Referenz-Hardware (zum Recherchieren, noch nicht final)

- ESP32 Low-Power-Boards (LOLIN32 Lite, FireBeetle ESP32-E)
- Kapazitive Bodenfeuchte-Sensoren v1.2 / v2.0
- Membranpumpen aus dem Aquaristik- bzw. Modellbau-Bereich
- TP4056 + Solarladeregler bzw. CN3791 (MPPT für 1S Li-Ion)
- ESPHome / Home Assistant für die optionale Smart-Home-Anbindung
