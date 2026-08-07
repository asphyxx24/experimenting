#!/usr/bin/env python3
"""Prueft, ob der Claude Agent SDK als Jarvis-Backend taugt.

Zwei Fragen, die ueber die Architektur entscheiden:

  1. Laeuft er ohne ANTHROPIC_API_KEY? Dann nutzt er dieselbe Anmeldung wie
     die CLI (Max-Abo) und nicht API-Billing. Falls nein, ist er fuer dieses
     Projekt unbrauchbar — genau wie `claude --bare`.
  2. Haelt ClaudeSDKClient den Kontext ueber mehrere Fragen, so wie der
     handgebaute stdin-Prozess in bench_stream.py?

Nebenbei wird dieselbe Latenz gemessen wie in bench_voice_latency.py, damit
die Zahlen vergleichbar sind.

Aufruf:  python agent_sdk_probe.py
"""

import asyncio
import os
import re
import time

from claude_agent_sdk import (
    AssistantMessage,
    ClaudeAgentOptions,
    ClaudeSDKClient,
    ResultMessage,
    TextBlock,
)

SYS_PROMPT = (
    "Du bist ein Sprachassistent. Deine Antworten werden vorgelesen. "
    "Antworte kurz, in hoechstens zwei Saetzen, in gesprochener Sprache "
    "ohne Aufzaehlungen, Ueberschriften oder Markdown-Formatierung."
)

PROMPTS = [
    "Was ist die Hauptstadt von Australien? Antworte in einem Satz.",
    "Und wie viele Einwohner hat sie ungefaehr?",
]

SENTENCE_END = re.compile(r"[.!?](\s|$)")


async def main():
    key = os.environ.get("ANTHROPIC_API_KEY")
    print(f"ANTHROPIC_API_KEY gesetzt: {bool(key)}")
    print("Wenn die Anfragen ohne Key durchlaufen, nutzt das SDK die "
          "CLI-Anmeldung (Max-Abo) statt API-Billing.\n")

    options = ClaudeAgentOptions(
        system_prompt=SYS_PROMPT,
        tools=[],                 # keine Tools -> keine Tool-Aufrufe zum Filtern
        setting_sources=[],       # Nutzer-Settings/CLAUDE.md nicht laden
        effort="low",
        include_partial_messages=True,
    )

    boot = time.perf_counter()
    async with ClaudeSDKClient(options=options) as client:
        print(f"Client bereit nach {(time.perf_counter()-boot)*1000:.0f} ms\n")

        for i, prompt in enumerate(PROMPTS, 1):
            sent = time.perf_counter()
            await client.query(prompt)

            first_text_at = None
            first_sentence_at = None
            buffer = ""
            answer = ""
            cost = None

            async for msg in client.receive_response():
                now = time.perf_counter()
                if isinstance(msg, AssistantMessage):
                    for block in msg.content:
                        if isinstance(block, TextBlock):
                            if first_text_at is None:
                                first_text_at = now
                            buffer += block.text
                            if (first_sentence_at is None
                                    and SENTENCE_END.search(buffer)):
                                first_sentence_at = now
                elif isinstance(msg, ResultMessage):
                    done_at = now
                    answer = str(getattr(msg, "result", "") or "")
                    cost = getattr(msg, "total_cost_usd", None)

            def ms(t):
                return (t - sent) * 1000 if t else 0.0

            print(f"Frage {i}: {prompt}")
            print(f"  erster Text {ms(first_text_at):.0f} ms | "
                  f"erster Satz {ms(first_sentence_at):.0f} ms | "
                  f"komplett {ms(done_at):.0f} ms"
                  + (f" | ${cost:.4f}" if cost else ""))
            print(f"  Antwort: {answer[:120]}\n")

    print("Kontext-Test: Wurde Frage 2 ('wie viele Einwohner hat SIE') "
          "korrekt auf Canberra bezogen?")


if __name__ == "__main__":
    asyncio.run(main())
