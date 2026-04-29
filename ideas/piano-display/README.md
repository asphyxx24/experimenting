# Piano-Display — Kabelloser Touch-Bildschirm fürs E-Piano

**Status:** 🔬 Recherche
**Letztes Update:** 2026-04-29

## Idee

Ein selbstgebautes 13"+ Touchdisplay das am E-Piano steht und analoge Notenzettel ersetzt. Primär als eigenständiges Gerät (Noten, YouTube, Web), optional auch als kabelloser Zusatzmonitor für den PC.

## Use Cases

- Noten und PDFs anzeigen, per Touch blättern
- MuseScore, Ultimate Guitar, web-basierte Notenblätter
- YouTube (Tutorials, Playalongs)
- Bei Bedarf: PC-Desktop drahtlos draufspiegeln (Spacedesk/VNC)
- Portabel — Akku für ~4–6 Stunden

## Hardware-Entscheidung

**Gewählt: Raspberry Pi 5 (4GB) als Standalone-Gerät** mit 13" Portable Touchscreen.

Begründung:
- RPi 5 schafft YouTube 1080p flüssig in Chromium — RPi 4 kämpft dabei
- Standalone bedeutet: unabhängig vom PC, jederzeit nutzbar
- Wireless-Monitor-Modus per Spacedesk/VNC trotzdem möglich
- Günstiger und mehr DIY-Spaß als fertiges Android-Tablet

## Modus-Switch

Zwei Hardware-Buttons im Gehäuse:

- **Button 1 → Standalone-Modus:** RPi läuft normal, eigene Apps, Chromium
- **Button 2 → Monitor-Modus:** RPi startet Spacedesk/VNC-Client, spiegelt PC-Desktop über WiFi

Beide Buttons hängen an GPIO-Pins des RPi und triggern ein Shell-Script.

### Erweiterung: Hardware-HDMI-Switch (optional, später)

Falls Wireless-Latenz im Monitor-Modus stört:
- HDMI-Switch-Modul (2 Ein → 1 Aus, ~8€) ins Gehäuse
- Eingang 1: RPi 5 HDMI
- Eingang 2: HDMI-Kabel vom PC
- Button schaltet physisch um, null Latenz

## Bauteile

| Teil | Spec | Preis | Bezug |
|------|------|-------|-------|
| Raspberry Pi 5 (4GB) | 4× Cortex-A76, 4GB RAM | ~70 € | Berrybase |
| 13" Portable USB-C Touchscreen | 1080p, Touch, dünn | ~150–180 € | Amazon / AliExpress |
| USB-C Powerbank 20.000 mAh | PD, gleichzeitig laden+entladen | ~40 € | Amazon |
| Mikro-Taster oder Side-Push | 2× für Modus-Switch | ~1 € | Reichelt |
| Gehäuse / Rückwand | 3D-Druck oder Acryl | ~10–15 € | Druckservice |
| USB-C Kabel (kurz, 15 cm) | RPi → Display | ~5 € | Amazon |
| HDMI-Switch-Modul (optional) | 2-in 1-out | ~8 € | AliExpress |

**Gesamt: ~275–310 €** (ohne optionalen HDMI-Switch)

## Architektur

```
[PC] ──WiFi──► [RPi 5] ──HDMI/USB-C──► [13" Touchscreen]
                  │                           │
                  │ Standalone-Modus:         │ Touch-Input
                  │ Chromium, MuseScore       ▼
                  │                    [GPIO Buttons]
                  │ Monitor-Modus:
                  └► Spacedesk/VNC-Client → PC-Desktop
```

## Software-Stack

- **OS:** Raspberry Pi OS (64-bit)
- **Browser:** Chromium → YouTube, Web-Noten
- **Noten offline:** MuseScore 4, PDF-Viewer
- **Wireless Monitor:** Spacedesk (kostenlos) oder VNC
- **Modus-Switch Script:** Python + RPi.GPIO, systemd-Service

## Trade-offs

| | Dieses Gerät | Android-Tablet (fertig) |
|--|-------------|------------------------|
| Netflix | ✗ | ✓ |
| YouTube | ✓ | ✓ |
| DIY / hackbar | ✓ | ✗ |
| PC-Monitor-Modus | ✓ (WiFi) | eingeschränkt |
| Preis | ~275–310 € | ~200–300 € |
| Aufwand | mittel | keiner |

## Offene Fragen

- [ ] Welcher konkrete 13" Touchscreen? (Waveshare, Elecrow, oder generischer USB-C-Monitor?)
- [ ] Powerbank: gleichzeitiges Laden + Betreiben (Pass-Through) prüfen — nicht alle können das
- [ ] Gehäuse: Rückwand mit Standfuß für Piano-Ablage, oder Clip-Montage am Piano-Ständer?
- [ ] Modus-Switch: reicht Spacedesk-Latenz, oder doch HDMI-Switch einplanen?

## Nächste Schritte

1. Konkreten 13" Touchscreen auswählen (Auflösung, Touch-Protokoll, USB-C PD)
2. RPi 5 + Display testen: YouTube, PDF-Viewer, MuseScore
3. Modus-Switch Script schreiben
4. Gehäuse designen und drucken
