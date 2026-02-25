# Modular Algorithmic Trading Engine (C++ + Python)

A modular algorithmic trading system that demonstrates how real-world trading engines are designed, tested, and evaluated before any live deployment.

This project combines **Python for market data ingestion** with a **high-performance C++ core** for indicator computation, strategy logic, risk management, and historical backtesting.  
The focus is on **system design, risk control, and correctness**, not prediction or hype.

---

## 📌 Features

- Automated market data fetching using Python
- Clean OHLCV candle normalization
- High-performance C++ trading engine
- Technical indicators:
  - SMA (Simple Moving Average)
  - EMA (Exponential Moving Average)
  - RSI (Relative Strength Index)
  - MACD
  - VWAP
  - ATR
- Score-based trading strategy
- ATR-based stop-loss and take-profit
- Historical backtesting with performance metrics
- Modular and extensible architecture

---

## 🧱 Project Structure
# Modular Algorithmic Trading Engine (C++ + Python)

A modular algorithmic trading system that demonstrates how real-world trading engines are designed, tested, and evaluated before any live deployment.

This project combines **Python for market data ingestion** with a **high-performance C++ core** for indicator computation, strategy logic, risk management, and historical backtesting.  
The focus is on **system design, risk control, and correctness**, not prediction or hype.

---

## 📌 Features

- Automated market data fetching using Python
- Clean OHLCV candle normalization
- High-performance C++ trading engine
- Technical indicators:
  - SMA (Simple Moving Average)
  - EMA (Exponential Moving Average)
  - RSI (Relative Strength Index)
  - MACD
  - VWAP
  - ATR
- Score-based trading strategy
- ATR-based stop-loss and take-profit
- Historical backtesting with performance metrics
- Modular and extensible architecture

---

## 🧱 Project Structure

trade/
│
├── backend/
│ └── data_fetcher.py # Python market data fetcher
│
├── data/
│ └── sample.csv # Normalized OHLCV data
│
├── cpp_engine/
│ ├── indicators.h
│ ├── indicators.cpp
│ ├── backtester.h
│ ├── backtester.cpp
│ ├── main.cpp
│ └── engine.exe # Compiled trading engine



---

## 🧠 Architecture Overview

- **Python layer**  
  Responsible only for fetching and preparing market data.

- **CSV interface**  
  Acts as a clean, language-agnostic data bridge.

- **C++ engine**  
  Handles all computation, strategy logic, risk management, and simulation.

The trading engine is **data-source agnostic**.  
Any market data provider can be plugged in by modifying only the Python layer.

---

## 🔄 How It Works

1. Python fetches recent market data (1-minute candles)
2. Data is normalized into OHLCV format
3. C++ engine loads candles from CSV
4. Indicators are calculated
5. Strategy logic generates trade signals
6. Trades are simulated using historical data
7. Stop-loss and take-profit are applied using ATR
8. Performance metrics are calculated and displayed

---

## 📈 Indicators Used

- **SMA** – Trend smoothing
- **EMA** – Faster trend detection
- **RSI** – Momentum and overbought/oversold conditions
- **MACD** – Trend and momentum confirmation
- **VWAP** – Volume-weighted fair price
- **ATR** – Volatility-based risk management

---

## 🛡️ Risk Management

- Stop-loss and take-profit are based on market volatility (ATR)
- Enforces favorable risk-to-reward ratios
- Tracks drawdown during backtesting
- Prevents uncontrolled losses

---

## 🧪 Backtesting

The backtesting engine simulates trades candle-by-candle using historical data.

Metrics produced:
- Total trades
- Wins and losses
- Net profit/loss
- Maximum drawdown

Backtesting ensures strategies are evaluated objectively before any live usage.

---

## ▶️ Build & Run

### 1. Fetch Market Data
From the project root:
```bash
python backend/data_fetcher.py
```

From inside the cpp_engine folder:
```bash
g++ main.cpp indicators.cpp backtester.cpp -o engine
engine.cpp
```

