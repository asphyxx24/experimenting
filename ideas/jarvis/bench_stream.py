#!/usr/bin/env python3
"""Misst einen DAUERHAFT laufenden `claude`-Prozess statt einen Start pro Frage.

Gegenstueck zu bench_headless.py: dort kostet jeder Aufruf den vollen
Prozess-Start. Hier laeuft ein Prozess durch und bekommt die Prompts per
stdin (--input-format stream-json). Wenn das funktioniert, faellt der
Start-Overhead genau einmal an — beim Booten des Servers, nicht bei jeder
Frage.

Aufruf:  python bench_stream.py
"""

import json
import subprocess
import sys
import threading
import time

SYS_PROMPT = (
    "Du bist ein Sprachassistent. Deine Antworten werden vorgelesen. "
    "Antworte kurz, in hoechstens zwei Saetzen, in gesprochener Sprache "
    "ohne Aufzaehlungen, Ueberschriften oder Markdown-Formatierung."
)

PROMPTS = [
    "Was ist die Hauptstadt von Australien? Antworte in einem Satz.",
    "Und wie viele Einwohner hat sie ungefaehr?",
    "Nenne mir eine bekannte Sehenswuerdigkeit dort.",
]

CMD = [
    "claude", "-p",
    "--input-format", "stream-json",
    "--output-format", "stream-json",
    "--verbose",
    "--tools", "",
    "--system-prompt", SYS_PROMPT,
    "--setting-sources", "",
    "--disable-slash-commands",
]


def main():
    print("Starte persistenten Prozess ...")
    boot_start = time.perf_counter()
    proc = subprocess.Popen(
        CMD, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, encoding="utf-8", bufsize=1,
    )

    events = []          # (timestamp, parsed_event)
    ready = threading.Event()

    def reader():
        # readline() statt `for line in proc.stdout` — die Iteration puffert
        # intern und liefert Zeilen erst verzoegert aus.
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
            events.append((time.perf_counter(), ev))
            if ev.get("type") == "system" and ev.get("subtype") == "init":
                print(f"  (Modell laut init: {ev.get('model')})")
                ready.set()

    threading.Thread(target=reader, daemon=True).start()

    # Nicht auf init warten, bevor gesendet wird: die CLI gibt das init-Event
    # erst aus, wenn sie stdin gelesen hat — sonst blockieren sich beide.
    boot_ms = (time.perf_counter() - boot_start) * 1000

    results = []
    for i, prompt in enumerate(PROMPTS, 1):
        msg = {
            "type": "user",
            "message": {"role": "user", "content": prompt},
        }
        before = len(events)
        sent = time.perf_counter()
        proc.stdin.write(json.dumps(msg) + "\n")
        proc.stdin.flush()

        # Auf das result-Event zu diesem Prompt warten
        deadline = sent + 60
        first_text_at = None
        result_at = None
        while time.perf_counter() < deadline:
            for ts, ev in events[before:]:
                if first_text_at is None and ev.get("type") == "assistant":
                    first_text_at = ts
                if ev.get("type") == "result":
                    result_at = ts
                    break
            if result_at:
                break
            time.sleep(0.01)

        if not result_at:
            print(f"Prompt {i}: kein result-Event — Abbruch")
            break

        ttft = (first_text_at - sent) * 1000 if first_text_at else 0
        total = (result_at - sent) * 1000
        results.append((ttft, total))
        # Antworttext mit ausgeben: nur so ist pruefbar, ob der Kontext
        # ueber mehrere Prompts hinweg erhalten bleibt.
        answer = ""
        for _, ev in events[before:]:
            if ev.get("type") == "result":
                answer = str(ev.get("result", ""))[:150]
        print(f"Prompt {i}: {total:.0f} ms bis fertig (erste Ausgabe nach {ttft:.0f} ms)")
        print(f"  Frage:   {prompt}")
        print(f"  Antwort: {answer}")

    try:
        proc.stdin.close()
        proc.wait(timeout=10)
    except Exception:
        proc.kill()

    if len(results) >= 2:
        # Prompt 1 traegt den Prozess-Start mit, Prompt 2+ nicht mehr.
        warm = [r[1] for r in results[1:]]
        print(f"\nErste Frage (inkl. Prozess-Start): {results[0][1]:.0f} ms")
        print(f"Folgefragen im laufenden Prozess:   {sum(warm)/len(warm):.0f} ms "
              f"(Einzelwerte: {', '.join(f'{w:.0f}' for w in warm)} ms)")
        print(f"Ersparnis pro Frage:                {results[0][1]-sum(warm)/len(warm):.0f} ms")


if __name__ == "__main__":
    main()
