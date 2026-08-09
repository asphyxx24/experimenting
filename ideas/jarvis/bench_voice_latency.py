#!/usr/bin/env python3
"""Misst die Latenz, die der Nutzer bei Jarvis tatsaechlich WAHRNIMMT.

bench_stream.py misst, wann die Antwort fertig ist. Fuer einen Sprach-
assistenten ist das die falsche Zahl: sobald der erste vollstaendige Satz
da ist, kann TTS zu sprechen anfangen. Alles danach laeuft im Hintergrund
nach. Dieses Skript misst deshalb drei Zeitpunkte pro Antwort:

  1. erstes Token        (Modell hat angefangen)
  2. erster ganzer Satz  <- ab hier koennte der Lautsprecher loslaufen
  3. Antwort komplett    (was bench_stream.py misst)

Zusaetzlich vergleichbar ueber --effort, um zu sehen, ob eine niedrigere
Denkstufe die Antwort spuerbar beschleunigt.

Aufruf:  python bench_voice_latency.py --effort low
"""

import argparse
import json
import re
import subprocess
import sys
import threading
import time

SYS_PROMPT = (
    "Du bist ein Sprachassistent. Deine Antworten werden vorgelesen. "
    "Antworte kurz, in hoechstens zwei Saetzen, in gesprochener Sprache "
    "ohne Aufzaehlungen, Ueberschriften oder Markdown-Formatierung."
)

# Bewusst Fragen, die zwei Saetze provozieren — bei einer einsaetzigen
# Antwort waere "erster Satz" und "komplett" dasselbe und die Messung
# wuerde den Streaming-Vorteil nicht zeigen.
PROMPTS = [
    "Erklaer mir kurz, warum der Himmel blau ist.",
    "Was kann ich bei Regen zuhause unternehmen? Nenne zwei Ideen.",
]

# Satzende: Punkt/Frage-/Ausrufezeichen, gefolgt von Leerzeichen oder Textende.
SENTENCE_END = re.compile(r"[.!?](\s|$)")


def build_cmd(effort):
    cmd = [
        "claude", "-p",
        "--input-format", "stream-json",
        "--output-format", "stream-json",
        "--include-partial-messages",
        "--verbose",
        "--tools", "",
        "--system-prompt", SYS_PROMPT,
        "--setting-sources", "",
        "--disable-slash-commands",
    ]
    if effort:
        cmd += ["--effort", effort]
    return cmd


def extract_text_delta(ev):
    """Findet den Text eines Streaming-Deltas, egal wie tief verschachtelt.

    Die CLI kapselt die rohen API-Events; statt die genaue Verschachtelung
    zu raten, wird rekursiv nach einem text_delta gesucht.
    """
    if isinstance(ev, dict):
        if ev.get("type") == "text_delta" and isinstance(ev.get("text"), str):
            return ev["text"]
        for value in ev.values():
            found = extract_text_delta(value)
            if found:
                return found
    return None


def run(effort, runs):
    proc = subprocess.Popen(
        build_cmd(effort), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, encoding="utf-8", bufsize=1,
    )

    events = []
    lock = threading.Lock()

    def reader():
        while True:
            line = proc.stdout.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue
            try:
                ev = json.loads(line)
            except json.JSONDecodeError:
                continue
            with lock:
                events.append((time.perf_counter(), ev))

    threading.Thread(target=reader, daemon=True).start()

    rows = []
    for i in range(runs):
        prompt = PROMPTS[i % len(PROMPTS)]
        with lock:
            start_index = len(events)
        sent = time.perf_counter()
        proc.stdin.write(json.dumps({
            "type": "user",
            "message": {"role": "user", "content": prompt},
        }) + "\n")
        proc.stdin.flush()

        first_token_at = None
        first_sentence_at = None
        done_at = None
        buffer = ""
        answer = ""
        deadline = sent + 90

        while time.perf_counter() < deadline:
            with lock:
                new = events[start_index:]
                start_index += len(new)
            for ts, ev in new:
                delta = extract_text_delta(ev)
                if delta:
                    if first_token_at is None:
                        first_token_at = ts
                    buffer += delta
                    if first_sentence_at is None and SENTENCE_END.search(buffer):
                        first_sentence_at = ts
                if ev.get("type") == "result":
                    done_at = ts
                    answer = str(ev.get("result", ""))
            if done_at:
                break
            time.sleep(0.005)

        if not done_at:
            print(f"  ! Lauf {i+1}: kein result-Event")
            continue

        def ms(t):
            return (t - sent) * 1000 if t else 0.0

        rows.append((ms(first_token_at), ms(first_sentence_at), ms(done_at)))
        print(f"  Lauf {i+1}: erstes Token {ms(first_token_at):.0f} ms | "
              f"erster Satz {ms(first_sentence_at):.0f} ms | "
              f"komplett {ms(done_at):.0f} ms")
        if not first_sentence_at:
            print("    (kein Satzende erkannt — Streaming-Deltas fehlen?)")
        print(f"    Antwort: {answer[:110]}")

    try:
        proc.stdin.close()
        proc.wait(timeout=10)
    except Exception:
        proc.kill()

    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--effort", default=None,
                    help="low | medium | high | xhigh | max (leer = CLI-Default)")
    ap.add_argument("--runs", type=int, default=2)
    args = ap.parse_args()

    label = args.effort or "default"
    print(f"== effort={label} ==")
    rows = run(args.effort, args.runs)
    if not rows:
        sys.exit("Keine Messwerte.")

    n = len(rows)
    avg = [sum(r[i] for r in rows) / n for i in range(3)]
    print(f"\nSchnitt (effort={label}, n={n}):")
    print(f"  erstes Token:   {avg[0]:.0f} ms")
    print(f"  erster Satz:    {avg[1]:.0f} ms   <- moeglicher TTS-Start")
    print(f"  komplett:       {avg[2]:.0f} ms")
    if avg[1]:
        print(f"  Ersparnis durch Streaming-TTS: {avg[2]-avg[1]:.0f} ms")


if __name__ == "__main__":
    main()
