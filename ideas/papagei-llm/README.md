# Papagei LLM

**Status:** 🔬 Recherche
**Letztes Update:** 2026-04-29

## Idee

Ein auf Verkaufsthemen (Kaltakquise mit Papagei) und Telefonie (insbesondere Inbound) spezialisiertes LLM, das wir self-hosten — primär um Latenzen durch externe API-Anbieter zu drücken und Domänen-Performance zu verbessern.

## Use Case

- **Outbound (Kaltakquise mit Papagei):** Begrüßung, Pitch, Einwandbehandlung, Qualifizierung, Termin-Booking.
- **Inbound-Telefonie:** Anrufannahme, Routing — konkrete Szenarien noch zu definieren.
- Sprache primär Deutsch.
- Pipeline ist klassisch STT → LLM → TTS (kein Voice-zu-Voice-Modell).

## Recherche-Ergebnisse

### Drei Ebenen, "eigene LLM zu bauen"

| Ebene | Was | Aufwand | Kosten | Realistisch für uns? |
|-------|-----|---------|--------|----------------------|
| 1 — Vortrainiertes Modell hosten + Prompt/RAG | Llama/Mistral/Qwen + eigene Daten via Vektorstore | Tage–Wochen | gering | ja |
| 2 — Finetuning (LoRA/QLoRA) | Open-Source-Modell auf eigene Gesprächsdaten nachtrainieren | Wochen–Monate | einige tausend € | ja, vermutlich der Weg |
| 3 — Pretraining from Scratch | Foundation Model selbst bauen | Jahre, Forscher-Team | 5–500 Mio. € | ❌ verworfen |

### Latenz — das eigentliche Treiber-Argument

Telefonie-Budget End-to-End: **<800ms komfortabel, <500ms premium.**

Pipeline-Glieder mit Richtwerten:

| Stufe | Realistisches Ziel |
|-------|--------------------|
| STT (Spracherkennung) | ~150 ms |
| LLM Time-to-First-Token | ~100 ms |
| LLM komplette kurze Antwort | ~300 ms |
| TTS (Sprachsynthese) | ~150 ms |
| Netzwerk + Telefonie | ~100 ms |

**Wichtig:** Der LLM-Teil ist nur ein Glied. Vor jeder Architekturentscheidung muss die heutige Pipeline pro Glied gemessen werden — sonst optimieren wir blind. Möglich ist, dass besseres TTS (ElevenLabs Flash ~75 ms, Cartesia Sonic ~90 ms, Deepgram Aura) und EU-Region für die LLM-API schon 80 % der Latenz lösen.

### Self-Hosting-Latenz realistisch

Ein gut deployedes 8B-Modell (Llama 3.1, Qwen 2.5) auf eigener H100/A100 mit vLLM oder TensorRT-LLM kann **TTFT ~50–100 ms** liefern — vs. OpenAI-API typischerweise ~300–500 ms. Das ist signifikant für Voice.

### Kandidaten-Basismodelle (Stand 2026-04)

- **Llama 3.1 8B** — solide im Deutschen, gut dokumentiert, große Community.
- **Qwen 2.5 7B** — sehr gute Multi-Sprach-Performance.
- **Mistral Nemo 12B** — speziell für europäische Sprachen optimiert.

### Daten — die unterschätzte Hürde

Für sinnvolles Finetuning brauchen wir:
- **Volumen:** 5.000–50.000 hochwertige Gesprächsbeispiele.
- **Format:** Transkript + gewünschte Antwort (Instruction-Tuning-Format).
- **Qualität:** anonymisiert, gelabelt, idealerweise mit Outcome (Termin ja/nein, Qualifiziert ja/nein).

Datenaufbereitung ist erfahrungsgemäß der zäheste Teil (3–6 Wochen reine Arbeit).

### Kosten-Schätzung Option C (Finetuning + Self-Hosting)

**Einmalig:**
- Datenaufbereitung: 5–20k €
- Finetuning-Compute (paar LoRA-Runs): 1–5k €
- Deployment-Setup, Monitoring: 5–15k €

**Laufend pro Monat:**
- GPU-Hosting (1× H100, redundant 2×): 3–8k €
- Monitoring, Updates, MLOps: 1–3k €
- **Gesamt: 4–11k €/Monat**

Break-Even gegen API-Kosten hängt am Call-Volumen — muss noch gerechnet werden.

### Compliance

- DSGVO Art. 6/9 für Gesprächsaufzeichnungen.
- §201 StGB (vertrauliches Wort) — Einwilligung der Angerufenen erforderlich.
- Spricht ohnehin für Self-Hosting / EU-Hosting.

## Entscheidungen

- ✅ **Pretraining from Scratch ist raus.** Kosten/Aufwand stehen in keinem Verhältnis zum Mehrwert.
- ✅ **Aktuell favorisiert:** Option C (Open-Source-Modell + LoRA-Finetuning + Self-Hosting), aber **erst nach Latenz-Audit**.
- ✅ **Voice-zu-Voice-Modelle (à la GPT-4o Realtime) bauen wir nicht selbst** — wir bleiben bei der STT → LLM → TTS Pipeline.

## Offene Fragen

- [ ] Welche STT/LLM/TTS-Anbieter nutzen wir aktuell, und wie ist die Latenz pro Glied?
- [ ] Wie viele parallele Calls fahren wir heute / planen wir? (bestimmt GPU-Bedarf)
- [ ] Welche konkreten Inbound-Szenarien wollen wir abdecken?
- [ ] Werden Gespräche heute schon aufgezeichnet & transkribiert? Wie viele Stunden Material liegen vor?
- [ ] Compliance-Lage geklärt (Einwilligung, Aufbewahrungsfristen)?
- [ ] **Build vs. Buy:** Lösen Sierra / Bland / Vapi / Retell das nicht schon?
- [ ] Ab welcher Call-Menge lohnt Self-Hosting gegenüber GPT-4o-mini-API?
- [ ] Wer im Team macht ML? Brauchen wir externen Consultant / ML-Engineer?
- [ ] Eval-Metriken: woran messen wir "besser"? (Conversion, Gesprächsdauer, manuelles Rating?)

## Nächste Schritte

### Phase 1 — Messen & verstehen (2–4 Wochen)
1. Aktuelle Latenz pro Pipeline-Glied messen (STT, LLM, TTS, Netzwerk).
2. Anbieter-Vergleich TTS (Latenz, Deutsch-Qualität, Preis).
3. Anbieter-Vergleich STT (Streaming, Deutsch).
4. Audit: welche Trainingsdaten haben wir heute?

### Phase 2 — Quick Wins (4–6 Wochen)
5. EU-Region für aktuelle LLM-API.
6. Bestes TTS einbauen.
7. Streaming end-to-end durchziehen.
8. **Entscheidungspunkt:** reicht das? → Stop, oder weiter zu Phase 3.

### Phase 3 — Self-Hosting OS-Modell (2–3 Monate)
9. Llama / Qwen / Mistral-Vergleich auf eigenen Test-Prompts.
10. vLLM-Deployment auf einer GPU.
11. A/B-Test gegen Phase-2-Lösung.
12. **Entscheidungspunkt:** Domänen-Qualität ausreichend? → Stop, oder weiter zu Phase 4.

### Phase 4 — Finetuning (3–6 Monate)
13. Daten-Pipeline aufbauen.
14. LoRA-Training, Iteration.
15. Eval-Suite (sales-spezifische Metriken).
16. Produktivsetzung.
