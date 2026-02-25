#pragma once
#include <vector>
#include <string>

// ─────────────────────────────────────────────
// Candle: one unit of market data (1 minute)
// ─────────────────────────────────────────────
struct Candle {
    std::string time;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

// ─────────────────────────────────────────────
// Indicator results (computed each tick)
// ─────────────────────────────────────────────
struct Indicators {
    double sma;
    double ema;
    double rsi;
    double macd;
    double macd_signal;
    double macd_hist;
    double vwap;
    double atr;
    bool   valid; // false if not enough candles yet
};

// ─────────────────────────────────────────────
// Function declarations
// ─────────────────────────────────────────────
double calcSMA(const std::vector<Candle>& candles, int period);
double calcEMA(const std::vector<Candle>& candles, int period);
double calcRSI(const std::vector<Candle>& candles, int period = 14);
void   calcMACD(const std::vector<Candle>& candles,
                double& macd, double& signal, double& hist);
double calcVWAP(const std::vector<Candle>& candles);
double calcATR(const std::vector<Candle>& candles, int period = 14);

Indicators computeAll(const std::vector<Candle>& candles);
