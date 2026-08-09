# Homelab — Arbeitskonventionen für KI-Sessions

Dieses Dokument gilt zusätzlich zu `experimenting/CLAUDE.md` (Repo-weite Konventionen). Es beschreibt, wie mit **diesem Projekt** gearbeitet werden soll — nicht, wie auf den Server zugegriffen wird (das ist aktuell explizit nicht vorgesehen, siehe README → "KI-gestützte Umsetzung").

## Dokumenten-Landkarte

- **`README.md`** — die vollständige Recherche- und Entscheidungsgrundlage. Vor jeder Aussage zum Projekt hier nachlesen, nicht raten.
- **`SETUP-PRP.md`** — der ausführbare Einrichtungsplan (Ziel, Kontext, Schritte, Validation Loop). Wird benutzt, wenn tatsächlich am Aufbau gearbeitet werden soll.
- **`llms.txt`** — Kurz-Index für schnelle Orientierung, welche Datei für welche Anfrage relevant ist.

## Umgang mit Status-Markierungen

In der README (v.a. Tabelle "Rollen" und `## Optional / Zurückgestellt`) haben Einträge einen Status:

- **`geplant`** — wird umgesetzt, Details ggf. noch offen.
- **`entschieden`** (im Fließtext markiert) — Diskussion ist abgeschlossen, Begründung steht dabei. Nicht ungefragt neu aufrollen oder in Frage stellen — nur wenn der Nutzer explizit sagt, dass sich etwas geändert hat.
- **`optional / zurückgestellt`** — bewusst geparkt (z. B. Trading Bot, Vaultwarden, Immich, UPS). Nicht von selbst aus wieder in aktive Planung heben oder in Vorschläge einbauen, es sei denn, der Nutzer fragt danach.

## Wie Vorschläge gemacht werden sollen

- Der Nutzer hat das technische Know-how, aber bewusst nicht die Detail-Wissensmenge zu jedem einzelnen Dienst, und möchte auch keine Zeit in deren Aneignung stecken. Erklärungen entsprechend knapp und handlungsorientiert halten, nicht mit Hintergrundwissen zutexten — außer explizit gefragt.
- Neue Ideen nur vorschlagen, wenn sie echten Mehrwert haben ("nicht auf Krampf") — der Nutzer hat das mehrfach explizit so gewünscht.
- Bei technischen Behauptungen (RAM-Bedarf, Kompatibilität, Preise) nach Möglichkeit konkret machen (Zahlen, Versionen), nicht vage bleiben — das ist der bisherige Standard in diesem Dokument.
- Wenn ein Widerspruch zwischen README-Inhalt und aktuellem Stand auffällt (z. B. veraltete Versionsangaben), aktiv melden und korrigieren, nicht stillschweigend übernehmen.
