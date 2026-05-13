# Trading Bot

Modularer, erweiterbarer Trading Bot für EU- und US-Aktien. Fokus auf technische Indikatoren und News-Sentiment. Startkapital ~100€ nach erfolgreichem Backtesting.

---

## Broker: Interactive Brokers (IBKR)

IBKR ist die zentrale Wahl — nicht Alpaca. Gründe:

- **Kein Pattern Day Trader Problem**: Die US-FINRA PDT-Regel (Account-Sperre bei <25.000$ und >3 Day-Trades/Woche) gilt für US-Broker. IBKR EU (irische Einheit) unterliegt europäischem Regelwerk.
- **Echter EU-Markt-Zugang**: Euronext, XETRA, LSE, NASDAQ — alles über eine API. Keine ADR-Umwege.
- **Paper Trading**: Kostenloses simuliertes Konto zum Testen.
- **API**: `ib_insync` Python-Library, stabil und gut dokumentiert.

> **Steuerhinweis**: IBKR EU führt keine deutsche Kapitalertragssteuer automatisch ab. Gewinne müssen manuell in der jährlichen Steuererklärung angegeben werden (Anlage KAP).

---

## Stack

| Bereich | Tool | Begründung |
|---|---|---|
| Broker | `ib_insync` (IBKR) | EU-konform, keine PDT-Regel, breiter Marktzugang |
| Backtesting Phase 1 | `vectorbt` | Schnell, vektorisiert, gut für Parameter-Sweeps |
| Backtesting Phase 2 | `backtrader` | Event-driven, realistisch mit Commissions + Slippage |
| Historische Daten | `yfinance` | Kostenlos, ausreichend für Backtesting |
| Indikatoren | `pandas-ta` | RSI, MACD, BB, ATR, EMA, Volume SMA |
| Sentiment Primär | Alpha Vantage `NEWS_SENTIMENT` | Gibt direkt Scores zurück, 25 req/day kostenlos |
| Sentiment Sekundär | `yfinance` News → FinBERT | Kostenlos, inoffiziell, als Fallback |
| Sentiment Tertiär | NewsAPI → FinBERT | 100 req/day kostenlos (~alle 15 Min) |
| Sentiment-Modell | `ProsusAI/finbert` | Auf Finanz-Headlines trainiert, richtig für diesen Use Case |
| Reasoning (optional) | OpenAI / Requesty API | Nur bei Bedarf für Edge Cases, nicht im Hot Path |
| Logging | SQLite via `sqlalchemy` | Lokal, kein Cloud-Overhead |
| Config | `.env` + `config.yaml` | Kein Hardcoding von Keys |
| Scheduler | `asyncio` Loop | Simpel, kein APScheduler-Overhead |

---

## Sentiment-Strategie: Cross-Validation

Alpha Vantage liefert eigene Sentiment-Scores. FinBERT läuft parallel über yfinance/NewsAPI-Headlines.

- Beide Quellen **stimmen überein** → höhere Confidence für das Signal
- Beide **widersprechen sich** → Confidence sinkt, Tendenz zu HOLD

Das reduziert False Positives ohne eine einzelne Quelle blind zu vertrauen.

---

## Architektur

```
bot/
├── main.py                     # Entry point, asyncio Scheduler
├── config.py                   # Lädt .env + config.yaml
├── data/
│   └── stock_feed.py           # yfinance Wrapper (OHLCV, historisch + live)
├── analysis/
│   ├── indicators.py           # pandas-ta Wrapper
│   ├── sentiment.py            # FinBERT + Alpha Vantage + NewsAPI Aggregator
│   └── reasoning.py            # OpenAI/Requesty API Wrapper (optional, nur für Edge Cases)
├── strategy/
│   ├── base_strategy.py        # Abstract Base Class
│   ├── momentum.py             # RSI + MACD + EMA Cross
│   └── sentiment_driven.py     # News-Sentiment getriggert
├── backtesting/
│   ├── vectorbt_runner.py      # Phase 1: schnelle Parameter-Optimierung
│   └── backtrader_runner.py    # Phase 2: realistische Simulation
├── execution/
│   ├── base_broker.py          # Abstract Base Class
│   └── ibkr_broker.py          # ib_insync Wrapper (Paper + Live)
├── risk/
│   └── manager.py              # Positionsgrößen, Stop-Loss, Drawdown-Check
└── logging/
    ├── db.py                   # SQLite Schema + Write-Funktionen
    └── reporter.py             # Trade-History + aktuelle Positionen
```

---

## Risk Manager — korrigierte Formel

```python
def position_size(capital, price, atr, multiplier=2.0, risk_per_trade=0.02) -> float:
    # Dollar at risk / Dollar stop distance = Units
    dollar_risk = capital * risk_per_trade
    stop_distance = atr * multiplier
    return dollar_risk / stop_distance  # Anzahl Units (nicht Kapital!)
```

Die ursprüngliche Formel (`risk_per_trade * capital / ATR`) fehlte die `multiplier`-Division und lieferte keine sinnvolle Unit-Anzahl. Korrigiert.

---

## Backtesting-Ablauf

1. **vectorbt**: Historische Daten via yfinance laden, Strategie-Parameter sweepen (z.B. RSI 10–20, EMA 20/50). Schnell, viele Kombinationen.
2. **backtrader**: Die besten Parameter aus Schritt 1 realistisch simulieren — mit Commissions (IBKR: ca. 0,05%, min. 1,25€) und Slippage.
3. Erst wenn backtrader profitable Ergebnisse zeigt → Paper Trading auf IBKR.
4. Nach stabilen Paper-Ergebnissen → Live mit ~100€.

---

## Wichtige Prinzipien

1. **Dry-run by default** — `dry_run: true` in config, echter Handel explizit opt-in
2. **Backtesting vor Paper Trading** — keine ungetestete Strategie live schalten
3. **Kein LLM im Hot Path** — FinBERT lazy-loaded, externes Reasoning-API nur on-demand
4. **Jede Entscheidung geloggt** — Signal, Confidence, Indikatoren, Sentiment-Scores, Reasoning
5. **Fehler crashen nicht den Bot** — try/except auf Feed- und Execution-Ebene
6. **Thread Safety** — `get_reasoning()` darf keinen State in der Strategie-Klasse speichern wenn mehrere Symbole parallel analysiert werden

---

## config.yaml Struktur

```yaml
general:
  dry_run: true
  log_level: INFO
  loop_interval_minutes: 60

markets:
  stocks:
    enabled: true
    symbols: []           # Zur Laufzeit befüllen, kein Hardcoding
    timeframe: "1d"

strategy:
  active: momentum        # momentum | sentiment_driven
  risk_per_trade: 0.02
  max_drawdown: 0.15

sentiment:
  alpha_vantage:
    enabled: true
    requests_per_day: 25
  newsapi:
    enabled: true
    requests_per_day: 100
  finbert:
    enabled: true
    model: ProsusAI/finbert

reasoning:
  enabled: false          # Nur aktivieren wenn Edge-Case-Analyse gewünscht
  provider: openai        # openai | requesty
  model: gpt-4o-mini      # Günstiges Modell reicht für diese Aufgabe
  use_for: edge_cases     # Nur bei widersprüchlichen Signalen aufrufen

ibkr:
  host: 127.0.0.1
  port: 7497             # 7497 = Paper Trading, 7496 = Live
  client_id: 1
```

---

## Milestones

### Milestone 1 — Backtesting
- `stock_feed.py` mit yfinance (historisch + live)
- `indicators.py` (RSI, MACD, BB, ATR, EMA, Volume SMA)
- `momentum.py` Strategie
- `risk/manager.py` (korrigierte Formel)
- `backtesting/vectorbt_runner.py`
- `backtesting/backtrader_runner.py`

### Milestone 2 — Paper Trading
- `ibkr_broker.py` gegen IBKR Paper Trading
- `db.py` Logging
- `main.py` mit asyncio Loop
- `sentiment.py` (Alpha Vantage + FinBERT Cross-Validation)

### Milestone 3 — Live
- Live-Switch in config (`port: 7496`)
- `reporter.py` für Trade-History
- `reasoning.py` optional aktivieren (OpenAI/Requesty für Edge Cases)
- Manuelle Steuer-Dokumentation einrichten

---

## Deployment: Dell OptiPlex 3090

Der Bot läuft permanent auf einem dedizierten Heimserver — nicht auf dem Haupt-Rechner.

**Hardware:** Dell OptiPlex 3090 — i5-10500T, 16GB DDR4, 256GB SSD, Windows 11

| Aspekt | Bewertung |
|---|---|
| CPU (i5-10500T, 35W TDP) | T-Variante = Low-Power, ideal für 24/7-Betrieb |
| RAM (16GB) | FinBERT-Inferenz braucht ~2GB, kein Problem |
| Stromkosten | ~15–25W unter Last, ca. 3–5€/Monat |
| FinBERT auf CPU | Funktioniert problemlos für stündliche Batch-Analyse |
| Externes Reasoning-API | Kein RAM-Problem, da API-Call statt lokalem Modell |

**Bot-Start via Windows Task Scheduler** — automatisch starten zu Marktöffnung, beenden nach Marktschluss:
- XETRA/Euronext: 09:00–17:30 Uhr
- NASDAQ: 15:30–22:00 Uhr

Der Bot muss also nicht wirklich 24/7 laufen, nur während der Marktzeiten.

**Speicherhinweis:** 256GB werden mit Emulations-ROMs, Cloud-Daten und Bot-Logs mittel­fristig eng. Externe SSD oder USB-NAS empfohlen — für den Start reicht es.

---

## Steuer

IBKR EU führt **keine** Kapitalertragssteuer automatisch ab. Alle realisierten Gewinne müssen jährlich manuell in der deutschen Steuererklärung angegeben werden (**Anlage KAP**, Abgeltungssteuer 26,375% inkl. Solidaritätszuschlag).

Empfohlen: IBKR-Jahresabschluss-Report herunterladen und dem Steuerberater / ELSTER übergeben.
