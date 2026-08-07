#!/usr/bin/env python3
"""Misst die Latenz von `claude -p` fuer den Jarvis-Sprachassistenten.

Frage, die das Skript beantwortet: Wie lange dauert eine Antwort vom
Knopfdruck bis zum ersten Ton — und wie viel davon ist Prozess-Start
(vermeidbar) statt Modell-Zeit (nicht vermeidbar)?

Aufruf:  python bench_headless.py [--runs 2]
"""

import argparse
import json
import shutil
import statistics
import subprocess
import sys
import time
import uuid

PROMPT = "Was ist die Hauptstadt von Australien? Antworte in einem Satz."
FOLLOWUP = "Und wie viele Einwohner hat sie ungefaehr?"

SYS_PROMPT = (
    "Du bist ein Sprachassistent. Deine Antworten werden vorgelesen. "
    "Antworte kurz, in hoechstens zwei Saetzen, in gesprochener Sprache "
    "ohne Aufzaehlungen, Ueberschriften oder Markdown-Formatierung."
)

# Jede Variante schaltet gegenueber der vorherigen eine Sache zu.
VARIANTS = [
    ("default",     []),
    ("no-tools",    ["--tools", ""]),
    ("lean",        ["--tools", "", "--system-prompt", SYS_PROMPT,
                     "--setting-sources", "", "--disable-slash-commands"]),
    ("lean+haiku",  ["--tools", "", "--system-prompt", SYS_PROMPT,
                     "--setting-sources", "", "--disable-slash-commands",
                     "--model", "haiku"]),
]


def run_once(extra_args, prompt):
    """Ein Aufruf. Gibt (wall_ms, parsed_json) zurueck."""
    cmd = ["claude", "-p", prompt, "--output-format", "json"] + extra_args
    start = time.perf_counter()
    proc = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8")
    wall_ms = (time.perf_counter() - start) * 1000
    if proc.returncode != 0:
        return wall_ms, {"is_error": True, "result": proc.stderr.strip()[:200]}
    try:
        return wall_ms, json.loads(proc.stdout)
    except json.JSONDecodeError:
        return wall_ms, {"is_error": True, "result": proc.stdout.strip()[:200]}


def measure(label, extra_args, runs):
    """Fuehrt eine Variante mehrfach aus und aggregiert."""
    rows = []
    for i in range(runs):
        wall, data = run_once(extra_args, PROMPT)
        if data.get("is_error"):
            print(f"  ! {label} Lauf {i+1} fehlgeschlagen: {data.get('result')}")
            continue
        # duration_ms = was die CLI intern gemessen hat (Modell + Netz).
        # Alles darueber hinaus ist Start-Overhead des Prozesses.
        inner = data.get("duration_ms", 0)
        rows.append({
            "wall_ms": wall,
            "inner_ms": inner,
            "overhead_ms": wall - inner,
            "ttft_ms": data.get("ttft_ms", 0),
            "cache_creation": data.get("usage", {}).get("cache_creation_input_tokens", 0),
            "cache_read": data.get("usage", {}).get("cache_read_input_tokens", 0),
            "cost_usd": data.get("total_cost_usd", 0.0),
        })
        print(f"  {label} Lauf {i+1}: {wall:.0f} ms wall "
              f"({inner:.0f} ms intern, {wall-inner:.0f} ms Overhead)")
    return label, rows


def measure_resume(extra_args, runs):
    """Misst den Folge-Prompt in derselben Session (Cache sollte warm sein)."""
    sid = str(uuid.uuid4())
    run_once(extra_args + ["--session-id", sid], PROMPT)  # Session anlegen
    rows = []
    for i in range(runs):
        wall, data = run_once(extra_args + ["--resume", sid], FOLLOWUP)
        if data.get("is_error"):
            print(f"  ! resume Lauf {i+1} fehlgeschlagen: {data.get('result')}")
            continue
        inner = data.get("duration_ms", 0)
        rows.append({
            "wall_ms": wall,
            "inner_ms": inner,
            "overhead_ms": wall - inner,
            "ttft_ms": data.get("ttft_ms", 0),
            "cache_creation": data.get("usage", {}).get("cache_creation_input_tokens", 0),
            "cache_read": data.get("usage", {}).get("cache_read_input_tokens", 0),
            "cost_usd": data.get("total_cost_usd", 0.0),
        })
        print(f"  resume Lauf {i+1}: {wall:.0f} ms wall "
              f"({inner:.0f} ms intern, {wall-inner:.0f} ms Overhead)")
    return "lean+resume", rows


def med(rows, key):
    vals = [r[key] for r in rows]
    return statistics.median(vals) if vals else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runs", type=int, default=2)
    args = ap.parse_args()

    if not shutil.which("claude"):
        sys.exit("claude-CLI nicht im PATH gefunden.")

    results = []
    for label, extra in VARIANTS:
        print(f"\n== {label} ==")
        results.append(measure(label, extra, args.runs))

    print("\n== lean+resume ==")
    lean_args = dict(VARIANTS)["lean"]
    results.append(measure_resume(lean_args, args.runs))

    print("\n\n| Variante | Wall (Median) | davon Modell | davon Start-Overhead | TTFT | Cache neu/gelesen | Kosten |")
    print("|---|---|---|---|---|---|---|")
    for label, rows in results:
        if not rows:
            print(f"| {label} | — fehlgeschlagen — | | | | | |")
            continue
        print(f"| {label} "
              f"| {med(rows,'wall_ms'):.0f} ms "
              f"| {med(rows,'inner_ms'):.0f} ms "
              f"| {med(rows,'overhead_ms'):.0f} ms "
              f"| {med(rows,'ttft_ms'):.0f} ms "
              f"| {med(rows,'cache_creation'):.0f} / {med(rows,'cache_read'):.0f} tok "
              f"| ${med(rows,'cost_usd'):.4f} |")


if __name__ == "__main__":
    main()
