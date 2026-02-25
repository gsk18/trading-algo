#include "indicators.h"
#include <numeric>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────
// SMA: average of last N closing prices
// ─────────────────────────────────────────────
double calcSMA(const std::vector<Candle>& candles, int period) {
    if ((int)candles.size() < period) return 0.0;
    double sum = 0.0;
    for (int i = candles.size() - period; i < (int)candles.size(); ++i)
        sum += candles[i].close;
    return sum / period;
}

// ─────────────────────────────────────────────
// EMA: exponentially weighted moving average
// multiplier = 2 / (period + 1), recent prices weighted more
// ─────────────────────────────────────────────
double calcEMA(const std::vector<Candle>& candles, int period) {
    if ((int)candles.size() < period) return 0.0;
    double multiplier = 2.0 / (period + 1);
    // seed EMA with SMA of first `period` candles
    double ema = 0.0;
    int start = candles.size() - period * 2; // go back further for accuracy
    if (start < 0) start = 0;
    // compute SMA seed
    double seed = 0.0;
    int seed_end = start + period;
    if (seed_end > (int)candles.size()) seed_end = candles.size();
    for (int i = start; i < seed_end; ++i) seed += candles[i].close;
    ema = seed / (seed_end - start);
    // apply EMA forward
    for (int i = seed_end; i < (int)candles.size(); ++i)
        ema = (candles[i].close - ema) * multiplier + ema;
    return ema;
}

// ─────────────────────────────────────────────
// RSI: momentum indicator (0-100)
// >70 = overbought, <30 = oversold
// ─────────────────────────────────────────────
double calcRSI(const std::vector<Candle>& candles, int period) {
    if ((int)candles.size() < period + 1) return 50.0; // neutral if not enough data
    double gain = 0.0, loss = 0.0;
    int start = candles.size() - period;
    for (int i = start; i < (int)candles.size(); ++i) {
        double change = candles[i].close - candles[i - 1].close;
        if (change > 0) gain += change;
        else            loss -= change;
    }
    if (loss == 0.0) return 100.0;
    double rs = (gain / period) / (loss / period);
    return 100.0 - (100.0 / (1.0 + rs));
}

// ─────────────────────────────────────────────
// MACD: EMA(12) - EMA(26), signal = EMA(9) of MACD
// ─────────────────────────────────────────────
void calcMACD(const std::vector<Candle>& candles,
              double& macd, double& signal, double& hist) {
    double ema12 = calcEMA(candles, 12);
    double ema26 = calcEMA(candles, 26);
    macd = ema12 - ema26;

    // To get signal line, we need a series of MACD values
    // Approximate with current MACD as signal if not enough history
    // For a proper signal, build a small MACD history
    int n = candles.size();
    if (n < 35) {
        signal = macd;
        hist   = 0.0;
        return;
    }

    // Build last 9 MACD values from sub-windows
    std::vector<double> macd_series;
    for (int offset = 9; offset >= 1; --offset) {
        std::vector<Candle> sub(candles.begin(), candles.end() - offset + 1);
        if ((int)sub.size() >= 26) {
            double m12 = calcEMA(sub, 12);
            double m26 = calcEMA(sub, 26);
            macd_series.push_back(m12 - m26);
        }
    }
    if (macd_series.empty()) {
        signal = macd; hist = 0.0; return;
    }

    // EMA(9) of the MACD series
    double mult = 2.0 / 10.0;
    signal = macd_series[0];
    for (int i = 1; i < (int)macd_series.size(); ++i)
        signal = (macd_series[i] - signal) * mult + signal;

    hist = macd - signal;
}

// ─────────────────────────────────────────────
// VWAP: volume-weighted average price
// resets each session (we use all available candles)
// ─────────────────────────────────────────────
double calcVWAP(const std::vector<Candle>& candles) {
    double tp_vol = 0.0, vol = 0.0;
    for (const auto& c : candles) {
        double typical = (c.high + c.low + c.close) / 3.0;
        tp_vol += typical * c.volume;
        vol    += c.volume;
    }
    return (vol == 0.0) ? 0.0 : tp_vol / vol;
}

// ─────────────────────────────────────────────
// ATR: average true range — measures volatility
// True Range = max(H-L, |H-prevClose|, |L-prevClose|)
// ─────────────────────────────────────────────
double calcATR(const std::vector<Candle>& candles, int period) {
    if ((int)candles.size() < period + 1) return 0.0;
    double atr = 0.0;
    int start = candles.size() - period;
    for (int i = start; i < (int)candles.size(); ++i) {
        double hl  = candles[i].high - candles[i].low;
        double hpc = std::abs(candles[i].high  - candles[i-1].close);
        double lpc = std::abs(candles[i].low   - candles[i-1].close);
        atr += std::max({hl, hpc, lpc});
    }
    return atr / period;
}

// ─────────────────────────────────────────────
// Compute all indicators in one call
// ─────────────────────────────────────────────
Indicators computeAll(const std::vector<Candle>& candles) {
    Indicators ind{};
    ind.valid = (candles.size() >= 30); // need at least 30 candles

    ind.sma  = calcSMA(candles, 20);
    ind.ema  = calcEMA(candles, 20);
    ind.rsi  = calcRSI(candles, 14);
    ind.vwap = calcVWAP(candles);
    ind.atr  = calcATR(candles, 14);
    calcMACD(candles, ind.macd, ind.macd_signal, ind.macd_hist);

    return ind;
}
