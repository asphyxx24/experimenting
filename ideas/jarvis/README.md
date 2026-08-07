# Jarvis — Sprachassistent auf eigenem Server

**Status:** 🔬 Recherche
**Letztes Update:** 2026-08-07

## Idee

Ein selbst gehosteter Sprachassistent auf dem [Homelab](../homelab/README.md)-OptiPlex: Knopfdruck → sprechen → Antwort hören. Kein Always-Listening, kein Cloud-Assistent, keine API-Rechnung — die Antwort kommt von Claude im Headless-Modus über das vorhandene Max-Abo.

Dies ist das **Backend-Gegenstück** zur [Companion-Watch](../companion-watch/README.md), die als Client bereits durchgeplant ist (dort "Jarvis-Backend" genannt, siehe deren Architektur-Diagramm und Spike D). Die Uhr ist bewusst als "dummer Client mit IO" definiert — die gesamte Intelligenz liegt hier.

## Architektur

```
Knopfdruck (Watch/Handy)
  → Mikro nimmt auf (Push-to-Talk, kein Always-Listening)
  → Audio an Server (Dell OptiPlex)
  → ASR (lokal, z. B. whisper.cpp)
  → Transkript als Prompt an Claude
  → Antwort per TTS abspielen (Boxen/Kopfhörer)
```

### Warum Push-to-Talk statt offenes Raummikro

- Kein Echo-/AEC-Problem, da Mikro nie gleichzeitig mit Lautsprecher aktiv ist
- Kein teures Fernfeld-Speakerphone nötig
- Weniger Datenschutz-/Fehlauslöse-Themen

Deckt sich mit der Client-Seite: die Companion-Watch hat PTT-Knopf als verlässlichste Eingabe bereits favorisiert (gegenüber Touchscreen-Hold und IMU-Geste).

### Claude-Anbindung (nutzt Max-Abo-Kontingent, nicht API-Billing)

**Entschieden nach Messung (2026-08-07, siehe [Messergebnisse](#messergebnisse-schritt-0)): ein dauerhaft laufender Prozess, kein Aufruf pro Frage — angebunden über den [Claude Agent SDK](https://code.claude.com/docs/en/agent-sdk), nicht über selbstgebautes CLI-Parsing.**

```python
from claude_agent_sdk import ClaudeAgentOptions, ClaudeSDKClient

options = ClaudeAgentOptions(
    system_prompt=SYS_PROMPT,   # Assistenten-Prompt statt Coding-Prompt
    tools=[],                   # keine Tools -> nichts zu filtern
    setting_sources=[],         # keine Nutzer-Settings/CLAUDE.md laden
    effort="low",
    include_partial_messages=True,   # Token-Deltas fuer Streaming-TTS
)

async with ClaudeSDKClient(options=options) as client:
    await client.query(transkript)
    async for msg in client.receive_response():
        ...   # Text an TTS weiterreichen
```

**Verifiziert (2026-08-07):** Der SDK läuft **ohne `ANTHROPIC_API_KEY`** — er nutzt dieselbe Anmeldung wie die CLI und damit das Max-Abo, nicht API-Billing. Das war die entscheidende offene Frage, weil `--bare` genau daran scheitert.

Der SDK ist derselbe Harness wie die CLI, bietet aber jede benötigte Option als Feld (`tools`, `system_prompt`, `setting_sources`, `effort`, `include_partial_messages`, `model`) statt als Flag-String — und erspart das Parsen des stdin/stdout-Protokolls. `ClaudeSDKClient` hält den Kontext über mehrere Fragen (verifiziert), `query()` wäre der Einzelaufruf.

Warum so:

- **Kein Prozess-Start pro Frage.** Der kostete gemessen 1,9–3,4 s und dominierte die Latenz. Beim laufenden Client fällt er einmalig an (~4,6 s Boot).
- **Kontext bleibt automatisch erhalten** — Session-IDs und `resume` werden dafür **nicht** gebraucht. Verifiziert: Auf "Was ist die Hauptstadt von Australien?" → "Und wie viele Einwohner hat *sie*?" kam korrekt die Einwohnerzahl von Canberra.
- **`tools=[]`** löst das "Tool-Aufrufe nicht vorlesen"-Problem an der Wurzel, statt sie nachträglich herauszufiltern.
- **`system_prompt`** ersetzt den Claude-Code-Systemprompt (~17,8k Token, auf Coding ausgelegt) durch eine kurze Assistenten-Anweisung — spart Kontingent und liefert sprechbare Antworten statt Markdown.
- **`setting_sources=[]`** lässt Nutzer-Settings und CLAUDE.md-Discovery weg. Nebeneffekt: ohne geladene Settings greift der Modell-Default (Sonnet) — wenn ein bestimmtes Modell gewollt ist, `model=` explizit setzen.

> **Zwei Sackgassen, beide geprüft:**
> - **`--bare`** würde den Start weiter beschleunigen (überspringt Hooks, LSP, Plugin-Sync, Auto-Memory), erzwingt laut CLI-Hilfe aber `ANTHROPIC_API_KEY` bzw. `apiKeyHelper` — OAuth und Keychain werden nie gelesen. Getestet: bricht mit *"Not logged in"* ab, `total_cost_usd: 0`, `duration_api_ms: 0` — es entsteht **kein** API-Request und damit auch keine Kosten. Ohne API-Guthaben schlicht unbenutzbar.
> - **Argument-Reihenfolge in der CLI:** `--tools` ist variadisch und schluckt nachfolgende Argumente. `claude -p --tools "" "Frage"` verliert den Prompt ("Input must be provided…"). Der Prompt gehört **vor** die variadischen Flags. Im SDK entfällt das Problem.

## Messergebnisse (Schritt 0)

Gemessen am 2026-08-07 auf dem Windows-Arbeitsrechner (nicht auf dem OptiPlex — die Zahlen sind Größenordnungen, keine Zielwerte), Claude Code 2.1.224. Skripte: [`bench_headless.py`](bench_headless.py) (ein Prozess pro Frage) und [`bench_stream.py`](bench_stream.py) (ein dauerhafter Prozess). Prompt jeweils: *"Was ist die Hauptstadt von Australien? Antworte in einem Satz."*

**Variante A — ein `claude -p`-Aufruf pro Frage** (Median aus je 2 Läufen):

| Variante | Gesamt | davon Modell | davon Prozess-Start | Kosten-Äquivalent |
|---|---|---|---|---|
| `default` (alle Tools, Opus) | 5869 ms | 2428 ms | **3441 ms** | $0,0496 |
| `--tools ""` (Opus) | 5885 ms | 2477 ms | **3408 ms** | $0,0101 |
| `lean` (eigener Systemprompt, ohne Settings/Skills → Sonnet) | 4036 ms | 2160 ms | **1875 ms** | $0,0018 |
| `lean` + `--model haiku` | 4720 ms | 2870 ms | 1850 ms | $0,0016 |
| `lean` + `--resume` (Folgefrage) | 3495 ms | 1553 ms | 1942 ms | $0,0014 |

**Variante B — ein dauerhaft laufender Prozess** (`--input-format stream-json`, Sonnet, 2 Läufe à 3 Prompts):

| | Zeit |
|---|---|
| Erste Frage (trägt den Prozess-Start mit) | ~3,0 s |
| Jede weitere Frage | **~1,6–2,6 s** (Schnitt ~2,1 s) |

**Variante C — Streaming-TTS und `effort`** ([`bench_voice_latency.py`](bench_voice_latency.py), persistenter CLI-Prozess mit `--include-partial-messages`, je 2 Läufe; der jeweils erste Lauf trägt den Prozess-Start mit):

| `effort` | erstes Token | **erster ganzer Satz** | Antwort komplett |
|---|---|---|---|
| `low` | 1945 ms | **2701 ms** | 3362 ms |
| `medium` | 1755 ms | **2874 ms** | 3332 ms |
| `high` | 1518 ms | **2787 ms** | 3171 ms |

Die mittlere Spalte ist die eigentlich interessante: Sobald der erste vollständige Satz da ist, kann TTS zu sprechen anfangen — der Rest läuft im Hintergrund nach. Der Abstand zur letzten Spalte ist der Gewinn: **383–661 ms, im Schnitt ~500 ms.**

**Variante D — Claude Agent SDK** ([`agent_sdk_probe.py`](agent_sdk_probe.py), `effort="low"`):

| | Zeit |
|---|---|
| Client-Boot (einmalig) | 4641 ms |
| Frage 1 komplett | 1526 ms |
| Frage 2 komplett (Folgefrage) | 2244 ms |

> **Einschränkung dieser Messung:** Das Probe-Skript wertet nur fertige Textblöcke aus, nicht die `StreamEvent`-Deltas — deshalb fallen dort "erster Text" und "komplett" fast zusammen. Das ist ein Artefakt des Testskripts, keine Aussage über den SDK; `include_partial_messages=True` ist gesetzt und die Deltas sind da, sie werden nur nicht ausgewertet. Für die Streaming-Zahlen gilt Variante C.

### Was daraus folgt

1. **Das Konzept trägt — aber nicht in der ursprünglich skizzierten Form.** Ein Prozess pro Äußerung verschenkt 1,9–3,4 s an reinen Startkosten. Der dauerhafte Prozess bringt die Antwortzeit auf ~2,1 s, und darin steckt fast nur noch echte Modellzeit.
2. **`--resume` und `--session-id` werden nicht gebraucht.** Der laufende Prozess hält den Kontext von selbst. Das vereinfacht die geplante Session-Verwaltung ersatzlos weg.
3. **Der Claude-Code-Systemprompt ist der größte Kostenposten.** ~17,8k Token pro Session, ausgelegt auf Coding. Mit eigenem `--system-prompt` fällt das Kosten-Äquivalent von $0,0496 auf $0,0018 pro Frage — Faktor 27. Beim Max-Abo zahlt man das nicht in Euro, aber im 5-Stunden-Kontingent.
4. **Haiku war nicht schneller als Sonnet** (4720 ms vs. 4036 ms, TTFT 2788 ms vs. 1982 ms). Bei n=2 nicht belastbar, aber genug, um "kleineres Modell = schnellere Antwort" nicht ungeprüft anzunehmen.
5. **Streaming-TTS ist der billigste verbleibende Hebel.** ~500 ms gefühlte Latenz, ohne irgendetwas am Modell zu ändern — einfach den ersten Satz sprechen, sobald er da ist. Setzt eine TTS-Engine voraus, die satzweise gefüttert werden kann (Piper kann das).
6. **`effort` ist *kein* Hebel.** Zwischen `low`, `medium` und `high` liegen 3362 / 3332 / 3171 ms — die Streuung zwischen zwei Läufen derselben Stufe ist größer als der Unterschied zwischen den Stufen. Bei so kurzen Antworten dominiert offenbar der Netzwerk- und Anlauf-Anteil. `low` kostet nichts und schadet nicht, aber es beschleunigt auch nichts Messbares.
7. **Der Agent SDK ist der richtige Weg für Schritt 2** — er nutzt nachweislich das Max-Abo, hält den Kontext, und macht das handgeschriebene stdin-Protokoll überflüssig. Der einmalige Boot von ~4,6 s ist beim Serverstart irrelevant.
8. **Offen bleibt das Gesamtbudget.** ~2 s sind nur das LLM-Glied. Dazu kommen ASR und TTS — erst deren Messung (Schritt 2) zeigt, ob die Gesamtlatenz akzeptabel ist.

> **Grenzen der Messung:** n=2 pro Variante, Windows statt Linux, Heimnetz-Verbindung, und die `lean`-Zeilen liefen auf Sonnet statt Opus (Nebeneffekt von `--setting-sources ""`) — der Vergleich `default` ↔ `lean` vermischt also Settings- und Modellwechsel. Für die getroffene Entscheidung reicht das, weil der Effekt (Prozess-Start als dominanter Posten) um ein Vielfaches größer ist als die Streuung. Auf dem OptiPlex nachmessen, wenn er steht.

## Hardware

| Zweck | Lösung | Hinweis |
|---|---|---|
| Server | Dell OptiPlex 3090 (vorhanden) | läuft ASR + Claude Code + TTS |
| Mikro (Übergang) | günstiges USB-Headset/Mikro | zum Testen, bevor Watch-Mikro steht |
| Mikro (Ziel) | Mikro der [Companion-Watch](../companion-watch/README.md) | Audio per WiFi/WebSocket an Server streamen (INMP441 I2S-Mikro, siehe deren `BAUTEILE.md`) |
| Lautsprecher | Xiaomi Soundbar 2.0 (AUX oder Bluetooth) | oder einfache günstige PC-Boxen |
| Konnektivität | USB-Bluetooth-Dongle (falls OptiPlex keins hat) | vorher prüfen: `lsusb` / `hciconfig` |

> **BT-Verfügbarkeit prüfen, nicht annehmen:** Der 3090 Micro hat einen M.2-2230-Slot ausschließlich für die WLAN/BT-Karte (siehe [Homelab → Hardware](../homelab/README.md#hardware)). Ob die Karte bestückt ist, entscheidet, ob der Dongle überhaupt nötig ist.

### Einkaufsliste

- [ ] USB-Bluetooth-Dongle (~10 €, nur falls keine WLAN/BT-Karte verbaut)
- [ ] Günstiges USB-Headset/Mikro (~10–15 €, Übergangslösung)
- [ ] 3,5mm-AUX-Kabel (Backup)
- [ ] Xiaomi Soundbar 2.0 (~60 €) oder simple PC-Boxen

## Berührungspunkte mit dem Homelab

Drei Punkte, an denen dieses Projekt die im Homelab dokumentierten Annahmen berührt:

1. **Dauerbetrieb.** Das Homelab hält aktuell fest: *"Aktuell kein Dauerbetrieb geplant"* (deshalb ist die USV zurückgestellt). Ein Assistent, der auf Knopfdruck antworten soll, setzt einen laufenden Server voraus. Entweder Dauerbetrieb wird zur Voraussetzung dieses Projekts erklärt — dann rückt die USV-Frage nach —, oder Jarvis lebt mit "Server muss an sein".
2. **RAM.** Die Worst-Case-Tabelle des Homelabs (~8,5–10 GB von 16 GB) enthält weder ASR noch TTS. Grober Zusatzbedarf, im Spike zu messen statt zu glauben: whisper.cpp `small` ~0,5 GB / `medium` ~1,5 GB residentes Modell, Piper-TTS ~0,1 GB. Headroom ist vorhanden, muss aber in die Rechnung.
3. **Audio-Ausgabe.** Bisher im Homelab gar kein Thema — der Server ist als headless Dienste-Knoten geplant. Bluetooth-Audio unter Linux (PipeWire) ist der fehleranfälligste Teil des Setups; AUX-Kabel ist der Fallback, der immer funktioniert.

## Alternative Architekturen

Vier Wege, die nicht gewählt sind, aber vor Schritt 2 bedacht gehören.

### Verworfen: Home Assistant Assist als Voice-Pipeline

**Entschieden (2026-08-07): Eigenbau. Nicht wieder aufrollen.**

Home Assistant bringt mit dem Wyoming-Protokoll eine fertige Voice-Pipeline mit (`faster-whisper` für ASR, Piper für TTS als eigene Container), und ESPHome hätte für die [Companion-Watch](../companion-watch/README.md) eine fertige ESP32-Voice-Komponente geliefert. Das hätte Audio-Transport, ASR, TTS und das Watch-Protokoll ersetzt.

**Bewusst nicht genommen — der Selbstbau ist hier der Zweck, nicht der Umweg.** Die Konsequenz ist eingepreist: ASR-Anbindung, TTS-Ausgabe, Netzwerk-Transport, das Watch-Protokoll und dessen Authentifizierung bleiben Eigenarbeit und damit offene Punkte dieser Idee. Die Wyoming-Bausteine selbst (`faster-whisper`, Piper) sind davon unberührt und können einzeln verwendet werden — verworfen ist die HA-Pipeline drumherum, nicht die Software darunter.

Nebenwirkung: Wenn Jarvis später Geräte steuern soll, läuft das über den ohnehin geplanten MQTT-Broker bzw. die HA-API als *Werkzeug*, nicht dadurch, dass HA die Sprachpipeline besitzt.

### Lokales Modell als Router und Fallback

Zwei Fälle, in denen Claude schlecht passt:

- **Kurzkommandos** ("Timer 5 Minuten", "Licht aus") brauchen kein Frontier-Modell. Ein lokales 8B-Modell (Ollama) antwortet in Millisekunden statt Sekunden. Ein Router davor entscheidet: Kommando → lokal, Rest → Claude.
- **Kontingent-Fallback:** Das Max-Abo hat ein 5-Stunden-Fenster, das sich Jarvis mit der Coding-Nutzung teilt. Ein lokales Modell hält den Assistenten am Leben, wenn das Fenster leer ist.

### Verworfen: Anthropic-API direkt

Kein API-Guthaben vorhanden, und der ganze Sinn des Projekts ist die Nutzung des vorhandenen Max-Abos. Betrifft auch alles, was implizit einen API-Key braucht — `--bare` und die meisten HA-Claude-Integrationen.

### Nicht geprüft: Pipecat / LiveKit Agents

Python-Frameworks für Echtzeit-Voice-Pipelines (VAD, Barge-in, Streaming-TTS). Für Push-to-Talk vermutlich überdimensioniert, aber die Streaming-TTS-Bausteine wären brauchbar, falls das Eigenbau-Skript unhandlich wird.

## Offene Fragen

- [ ] **Latenz-Budget:** Was ist "schnell genug"? Anders als bei [papagei-llm](../papagei-llm/README.md) (Telefonie, <800 ms) darf ein persönlicher Assistent vermutlich 2–4 s brauchen — das entscheidet, ob whisper.cpp `medium` auf CPU reicht oder `small` nötig ist.
- [ ] **TTS-Engine:** Piper (lokal, schnell, ordentliches Deutsch) vs. Cloud-TTS (bessere Stimme, aber Datenabfluss + Kosten). Piper ist der naheliegende Default und passt zur Datenhoheits-Motivation des Homelabs.
- [ ] **Session-Handling:** Der laufende Prozess hält den Kontext unbegrenzt — offen bleibt, *wann zurückgesetzt* wird (nachts? auf Zuruf "vergiss das"?) und was bei Prozess-Absturz passiert (Neustart = Kontext weg, ~3 s Anlaufzeit für die erste Frage).
- [ ] **Auth Gerät ↔ Backend:** offen — steht identisch in der Companion-Watch-README. Sollte einmal entschieden und dort wie hier gleich beantwortet werden.
- [ ] **Mehrere Client-Geräte** (Handgelenk, Schreibtisch, Handy): wie unterscheidet das Backend sie, und teilen sie sich eine Claude-Session?
- [ ] **Transport zum Client:** WebSocket dauerhaft offen vs. Pull bei PTT — ebenfalls offen in der Companion-Watch.
- [ ] **Was darf Jarvis?** Nur reden, oder auch Home Assistant / MQTT steuern? Letzteres wäre der eigentliche Mehrwert gegenüber "Chat mit Mikro", zieht aber Berechtigungsfragen nach sich.

## Nächste Schritte

### ~~Schritt 0 — Claude-Headless messen~~ ✅ erledigt (2026-08-07)

Ergebnis siehe [Messergebnisse](#messergebnisse-schritt-0): Architektur trägt, aber als dauerhafter Prozess statt als Aufruf pro Frage. ~2,1 s pro Antwort. Nachmessen auf dem OptiPlex, sobald der steht.

### Schritt 1 — Audio unter Linux

Bluetooth/USB-Audio auf dem OptiPlex zum Laufen bringen (Aufnahme **und** Wiedergabe). AUX als Fallback bereithalten.

### Schritt 2 — Pipeline als Python-Skript

Aufnahme (knopfdruck-getriggert) → whisper.cpp → `ClaudeSDKClient` (siehe Claude-Anbindung) → TTS-Ausgabe **satzweise**, sobald der erste Satz steht. Erstmal ein Skript auf dem Server mit USB-Headset, kein Netzwerk, kein Client. [`agent_sdk_probe.py`](agent_sdk_probe.py) ist die Vorlage für die LLM-Anbindung, die Satz-Erkennung aus [`bench_voice_latency.py`](bench_voice_latency.py) für den TTS-Trigger. Dabei ASR- und TTS-Latenz mitmessen — dann steht das Gesamtbudget.

> ASR, LLM und TTS als drei getrennte Funktionen schreiben — nicht wegen eines möglichen Umstiegs auf eine Fertiglösung (die ist verworfen), sondern damit einzelne Glieder austauschbar bleiben, wenn die Messung zeigt, wo der Engpass sitzt.

### Schritt 3 — Netzwerk-Client

Pipeline hinter eine kleine Netzwerk-Schnittstelle legen, damit ein entferntes Gerät Audio schicken kann. Ab hier ist das Handy als Client testbar.

### Schritt 4 — Companion-Watch als Eingabequelle

Zusammenführung mit [Spike D der Companion-Watch](../companion-watch/README.md#spike-d--jarvis-voice-integration-23-wochen). Setzt voraus, dass die Uhr-Hardware steht (Spikes 0–C).
