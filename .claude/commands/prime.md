---
description: Lädt Repo-Konventionen und Ideen-Index (ohne einzelne Ideen zu öffnen)
---

# Prime — Repo-Kontext laden

Du arbeitest in einem Ideen-Sammlung-Repo. Lade in dieser Reihenfolge nur die **Konventionen** und den **Ideen-Index**, **nicht** die einzelnen Ideen-READMEs.

## Schritt 1 — Konventionen lesen

Lies diese Dateien, um Layout & Arbeitsweise zu verstehen:

- `CLAUDE.md` (Repo-weite Regeln; ist meist schon geladen, aber zur Sicherheit nochmal kurz prüfen)
- `ideas/README.md` (Ideen-Index mit Status-Legende und Tabelle aller Ideen)

## Schritt 2 — Projektstruktur scannen (nur Pfade, kein Inhalt)

Verschaffe dir einen Überblick, **welche** Ideen-Ordner es gibt — ohne ihre READMEs zu lesen:

```
ls ideas/
```

Optional: pro Ideen-Ordner kurz prüfen, ob es eine eigene `CLAUDE.md` gibt (die wäre relevant, sobald wir in der Idee arbeiten) — aber **nicht** die `README.md` der Ideen öffnen.

## Schritt 3 — Was du dir merkst

- Die **Konventionen** aus `CLAUDE.md` und `ideas/README.md` (Layout, Status-Legende, Workflow für neue Ideen).
- Die **Namen und Status** aller existierenden Ideen aus dem Index — nicht mehr.
- Dass Details zu einzelnen Ideen in `ideas/<name>/README.md` liegen und **erst auf Anfrage** gelesen werden.

## Schritt 4 — Was du NICHT tust

- Keine `ideas/*/README.md` öffnen, solange der User nicht über eine konkrete Idee redet oder es explizit verlangt.
- Keine Zusammenfassungen einzelner Ideen erstellen.
- Keine Annahmen über Inhalte der Ideen treffen — nur die Existenz und der Status sind bekannt.

## Schritt 5 — Kurz bestätigen

Antworte am Ende in 2–3 Zeilen:

1. „Konventionen geladen" + ein Satz zum Layout.
2. Liste der bekannten Ideen-Namen mit Status (aus dem Index, ohne Details).
3. Hinweis: „Für Details zu einer Idee bitte den Namen nennen — dann lese ich die README."
