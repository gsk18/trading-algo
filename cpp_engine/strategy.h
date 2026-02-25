#pragma once
#include "indicators.h"
#include <string>

// ─────────────────────────────────────────────
// Signal types
// ─────────────────────────────────────────────
enum class Signal {
    STRONG_BUY,
    BUY,
    HOLD,
    SELL,
    STRONG_SELL
};

struct TradeAdvice {
    Signal      signal;
    std::string label;       // "STRONG BUY" etc.
    int         score;       // raw score from strategy
    double      stop_loss;
    double      take_profit;
    double      risk_reward; // take_profit distance / stop_loss distance
};

TradeAdvice evaluate(const Candle& latest, const Indicators& ind);
std::string signalColor(Signal s); // ANSI color code
