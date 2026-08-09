# Minecraft Controller-Mod (Couch-Setup)

Controller-Support für Minecraft Java (Hypixel Skyblock) am Mini-PC (OptiPlex) auf der Couch. Ziel: bequemes Spielen mit Controller statt Maus/Tastatur, inkl. Nutzung der Rücken-Paddles/Trigger eines Scuf-artigen Controllers, die aktuell nicht erkannt werden.

**Status: Recherche abgeschlossen, Entscheidung getroffen — Umsetzung offen.**

---

## Entscheidung: Controlify statt Controllable

| | **Controlify** (gewählt) | Controllable |
|---|---|---|
| Entwickler | isXander | MrCrayfish |
| Loader | Fabric, NeoForge | Fabric, NeoForge |
| MC-Version | 1.21.1+ | 1.20.1+ |
| Input-Lib | SDL3 (moderner) | SDL2 |
| Lizenz | LGPL-3.0 | MIT + Zlib |
| Aktivität | sehr aktiv, bis in aktuelle Snapshots gepflegt | aktiv, älterer Unterbau, 19,4 Mio. Downloads |
| Custom-Bindings | Data-driven, dokumentiertes Wiki (moddedmc.wiki), Resourcepacks können Bindings/Glyphen erweitern | GameControllerDB-basiert, Contributor-Doku laut Repo als "TODO" markiert |
| Extras | Gyro-Aim, adaptive Trigger/Haptik (DualSense), volle GUI-Navigation | Radialmenüs |

**Begründung:** Modernere Codebasis (SDL3), aktiver gepflegt, klarer dokumentiert für Custom-Bindings — passt besser zu geplanten eigenen Erweiterungen. Beide sind reine Client-Mods, keine bekannten Inkompatibilitäten mit Hypixel Skyblock.

Quellen: [Controlify GitHub](https://github.com/isXander/Controlify), [Controllable GitHub](https://github.com/MrCrayfish/Controllable), [Controlify Modrinth](https://modrinth.com/mod/controlify)

---

## Problem: Rücken-Paddles/Trigger werden nicht erkannt

Controller hat zwei zusätzliche Rücken-Trigger (Scuf-artig), die im aktuell genutzten Mod (unbekannt welcher) nicht erkannt werden.

Beide Mods bauen auf SDL auf, das intern eine `gamecontrollerdb.txt` nutzt, um Rohbuttons einem Standard-Layout zuzuordnen. Zwei Lösungswege, je nach Ursache:

1. **SDL2 Gamepad Tool** ([gamepad-tool.threeeyedgames.com](https://gamepad-tool.threeeyedgames.com)) — alle Buttons des Controllers durchklicken, inkl. Paddles. Tool erzeugt eine Mapping-Zeile, die man manuell in die `gamecontrollerdb.txt` im Mod-Config-Ordner einträgt.
2. **Falls SDL die Paddles gar nicht als Buttons sieht** (proprietäres Extra-Signal statt Standard-Button): Steam Input als Zwischenschicht. Minecraft-Launcher als "Nicht-Steam-Spiel" in Steam hinzufügen — Steam Input mappt beliebige Rohsignale auf virtuelle Xbox-Buttons, die SDL/Controlify dann normal erkennen.

**Nächster Schritt:** Erst Weg 1 probieren (10 Minuten, keine Toolchain nötig), erst bei Scheitern Weg 2.

---

## Eigene Anpassung / Fork (optional)

Rechtlich unproblematisch: Controlify steht unter **LGPL-3.0**, erlaubt Modifikation und private Nutzung ohne Auflagen. Erst bei Veröffentlichung der modifizierten Version müsste der eigene Quellcode ebenfalls unter LGPL offengelegt werden.

**Technischer Weg, falls Button-Remapping nicht reicht** (z.B. eigene Radial-Menü-Logik, neue Aktionen):
1. Repo klonen: `github.com/isXander/Controlify`
2. Standard-Toolchain: Java + Gradle, Fabric Loom (Gradle-Wrapper bringt das mit)
3. `./gradlew runClient` startet dev-Minecraft-Instanz direkt mit der modifizierten Mod — kein Umweg über Mod-Ordner
4. Änderungen im Input-Handling-Code vornehmen

**Einschätzung:** Lohnt sich erst, wenn die SDL-Mapping-Route nicht reicht. Für reines Paddle-Remapping ist der Fork Overkill.

---

## Offene Fragen

- [ ] Welcher Mod läuft aktuell überhaupt (unbekannt)? → vor Umstieg auf Controlify klären/deinstallieren
- [ ] Werden die Paddles vom OS/SDL überhaupt als Rohsignal erkannt? → mit SDL2 Gamepad Tool testen
- [ ] Falls Fork nötig: welche konkrete Zusatzfunktion soll rein?
