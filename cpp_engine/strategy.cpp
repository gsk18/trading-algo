#include "strategy.h"
#include <cmath>

// ─────────────────────────────────────────────
// Score-based strategy
// Each condition votes +/- on a running score
// Score maps to a signal
// ─────────────────────────────────────────────
TradeAdvice evaluate(const Candle& latest, const Indicators& ind) {
    int score = 0;
    double price = latest.close;

    // ── Trend conditions ──────────────────────
    if (price > ind.sma)          score += 1;  // price above 20 SMA → bullish
    if (price > ind.ema)          score += 1;  // price above 20 EMA → bullish
    if (ind.ema > ind.sma)        score += 1;  // EMA > SMA → uptrend forming

    // ── Momentum (RSI) ───────────────────────
    if (ind.rsi < 30)             score += 3;  // deeply oversold → strong buy opportunity
    else if (ind.rsi < 40)        score += 1;  // slightly oversold
    else if (ind.rsi > 70)        score -= 3;  // overbought → avoid / sell
    else if (ind.rsi > 60)        score -= 1;  // approaching overbought

    // ── MACD ─────────────────────────────────
    if (ind.macd > ind.macd_signal) score += 2; // MACD crossed above signal → bullish
    else                            score -= 1;  // MACD below signal → bearish pressure
    if (ind.macd_hist > 0)          score += 1;  // positive histogram → momentum building
    
    // ── VWAP ─────────────────────────────────
    if (price > ind.vwap)           score += 1;  // above VWAP → institutions buying
    else                            score -= 1;  // below VWAP → selling pressure

    // ── Determine signal ─────────────────────
    Signal sig;
    std::string label;

    if      (score >= 7)  { sig = Signal::STRONG_BUY;  label = "STRONG BUY";  }
    else if (score >= 3)  { sig = Signal::BUY;         label = "BUY";         }
    else if (score >= -2) { sig = Signal::HOLD;        label = "HOLD";        }
    else if (score >= -5) { sig = Signal::SELL;        label = "SELL";        }
    else                  { sig = Signal::STRONG_SELL; label = "STRONG SELL"; }

    // ── Risk management (ATR-based) ───────────
    double atr = (ind.atr > 0) ? ind.atr : price * 0.005; // fallback: 0.5% of price
    double stop_loss   = price - (1.5 * atr); // risk 1.5x ATR below entry
    double take_profit = price + (3.0 * atr); // reward 3x ATR above entry (2:1 RR)
    double sl_dist = price - stop_loss;
    double tp_dist = take_profit - price;
    double rr = (sl_dist > 0) ? (tp_dist / sl_dist) : 0.0;

    return TradeAdvice{sig, label, score, stop_loss, take_profit, rr};
}

// ─────────────────────────────────────────────
// ANSI terminal colors for the dashboard
// ─────────────────────────────────────────────
std::string signalColor(Signal s) {
    switch (s) {
        case Signal::STRONG_BUY:  return "\033[1;32m"; // bold green
        case Signal::BUY:         return "\033[32m";   // green
        case Signal::HOLD:        return "\033[33m";   // yellow
        case Signal::SELL:        return "\033[31m";   // red
        case Signal::STRONG_SELL: return "\033[1;31m"; // bold red
    }
    return "\033[0m";
}
