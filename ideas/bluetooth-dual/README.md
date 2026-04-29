# Bluetooth Dual Audio

**Status:** 🔬 Recherche
**Letztes Update:** 2026-04-29

## Idee

Ein Handy spielt Musik (Spotify, YouTube Music, ...) und streamt den Audio-Output gleichzeitig an zwei unterschiedliche Bluetooth-Kopfhörer — z. B. um zu zweit Musik zu hören, ohne ein Kabel oder einen Splitter zu brauchen.

## Use Case

- Person A startet auf ihrem Handy Musik (Spotify oder YouTube Music).
- Person A und Person B hören parallel auf ihren eigenen Bluetooth-Kopfhörern mit.
- Mal kommt der Stream vom Handy von A, mal von B.
- Hardware: Samsung Galaxy S23 + Pixel 7 Pro.

## Recherche-Ergebnisse

### Standard-Lösungen, die schon existieren

- **Android Dual Audio (A2DP)**: ab Android 8 nativ verfügbar. Zwei A2DP-Sinks parallel. Funktioniert auf S23 (Samsung "Dual Audio") und Pixel 7 Pro (zwei Geräte verbinden + beide aktiv lassen).
- **Apple "Audio teilen"**: nur für Apple-Hardware, hier irrelevant.
- **Bluetooth LE Audio + Auracast**: der eigentlich richtige Standard dafür. Broadcast Audio an beliebig viele Empfänger, latenz-synchron. Voraussetzung: Sender + Empfänger müssen LE Audio können (Pixel 8+, Galaxy S23+ teilweise via Update, neue Sony/JBL/Bose-Modelle).

### Bewertung Dual A2DP für unseren Use Case

**Pro:**
- Funktioniert mit beliebigen Bluetooth-Kopfhörern (anders als Apple Audio Share).
- Audio aus jeder App.
- Lautstärke pro Kopfhörer separat regelbar.
- Beide Handys (S23, Pixel 7 Pro) unterstützen es nativ.

**Contra:**
- Latenz-Drift zwischen den beiden Kopfhörern (~20–150 ms). Für reine Musik nebeneinander nicht relevant. Bei Video problematisch.
- Codec-Downgrade auf SBC, sobald zwei Sinks aktiv sind. aptX/LDAC/AAC oft nur mit *einem* Sink.
- Reichweite/Stabilität leidet bei zwei parallelen Streams.

### Wie schlimm ist SBC?

Kurz: für unseren Use Case nicht relevant.

- Spotify Premium streamt 320 kbps Ogg Vorbis, YouTube Music ~256 kbps AAC — Quellmaterial ist eh schon komprimiert.
- Auf In-Ears < 200 € in Blindtests kaum unterscheidbar von aptX.
- Auf High-End Over-Ears in Stille mit gut produzierter Musik: leichte Verluste in Höhen / dichten Mixes hörbar, aber nicht "kaputt".
- Im Alltag (Café, Bahn, Spaziergang) → Null Unterschied.

**Tweak**: in Android-Entwickleroptionen lässt sich SBC-Bitrate / Sample Rate manchmal hochstellen (44.1 kHz Sample Rate ist optimal).

## Entscheidung

**Erst das Standard-Feature testen, bevor wir entwickeln.**

Mit S23 + Pixel 7 Pro ist Dual A2DP nativ verfügbar und sollte für den beschriebenen Workflow ausreichen. Konkret:

1. Auf dem Sender-Handy beide Kopfhörer pairen.
2. Dual Audio aktivieren (Samsung: Einstellungen → Bluetooth → Erweitert → Dual Audio. Pixel: einfach beide verbunden lassen).
3. Spotify / YouTube Music starten und prüfen, ob Latenz und Klang akzeptabel sind.

## Eigenes Projekt — falls das Standard-Feature nicht reicht

Drei sinnvolle Ansätze, falls wir trotzdem etwas bauen wollen:

### 1. App-Ebene (Android)
- App, die Dual A2DP komfortabler macht: schnelles Pairing, Latenz-Ausgleich, geteilte Playlist-Steuerung.
- Limitiert durch das, was die OS-API hergibt. Kein "echtes" technisches Problem zu lösen.

### 2. Wirklich neu: zwei Streams aus zwei Apps gleichzeitig
- Person A hört Spotify auf Kopfhörer 1, Person B hört YouTube Music auf Kopfhörer 2 — alles vom selben Handy.
- Das gibt es **nicht** als Out-of-the-Box-Feature. Hier wäre echter Mehrwert.
- Erfordert tiefes Eingreifen in Android Audio Routing (Audio Policy Service). Ohne Root oder eigenen ROM sehr eingeschränkt machbar.

### 3. Hardware-Splitter
- ESP32 mit `ESP32-A2DP` (pschatzmann) oder Dual-A2DP-Chip (CSR8675/QCC).
- Empfängt einen Stream vom Handy, sendet an zwei Kopfhörer weiter.
- Ehrlicher Weg als Software, aber zusätzliches Gerät unterwegs.

### 4. Auracast (zukunftssicher)
- Wenn Hardware das mal alle kann, ist es genau die richtige Lösung.
- Heute noch zu fragmentiert.

## Offene Fragen

- [ ] Latenz auf S23 und Pixel 7 Pro mit unseren tatsächlichen Kopfhörern messen.
- [ ] Klingt SBC mit den konkreten Kopfhörern gut genug?
- [ ] Ist das Feature auf beiden Handys gleich komfortabel zu bedienen, oder wäre eine kleine "Quick-Toggle"-App nützlich?
- [ ] Wenn wir doch entwickeln: wäre Variante 2 (zwei Apps → zwei Kopfhörer) realistisch ohne Root?

## Nächste Schritte

1. Dual A2DP auf beiden Handys testen.
2. Falls okay → Idee als gelöst markieren, ggf. archivieren.
3. Falls nicht → Variante 2 oder 3 evaluieren.
