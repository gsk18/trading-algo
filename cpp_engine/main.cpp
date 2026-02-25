#include "indicators.h"
#include "strategy.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ctime>

bool parseCandle(const std::string& line, Candle& out) {
    if (line.empty() || line[0] == '#') return false;
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> fields;
    while (std::getline(ss, token, ','))
        fields.push_back(token);
    if (fields.size() < 6) return false;
    try {
        out.time   = fields[0];
        out.open   = std::stod(fields[1]);
        out.high   = std::stod(fields[2]);
        out.low    = std::stod(fields[3]);
        out.close  = std::stod(fields[4]);
        out.volume = std::stod(fields[5]);
    } catch (...) { return false; }
    return true;
}

// Plain-text log writer — appends each candle's result, never overwrites
void writeLog(std::ofstream& log, const Candle& c,
              const Indicators& ind, const TradeAdvice& advice) {
    log << "========================================\n";
    log << "  Time    : " << c.time << "\n";
    log << std::fixed << std::setprecision(4);
    log << "  O: " << c.open << "  H: " << c.high
        << "  L: " << c.low  << "  C: " << c.close
        << "  Vol: " << (long long)c.volume << "\n\n";

    log << "  INDICATORS\n";
    log << "  SMA(20)  : " << std::setw(10) << ind.sma << "\n";
    log << "  EMA(20)  : " << std::setw(10) << ind.ema << "\n";
    log << "  RSI(14)  : " << std::setw(10) << std::setprecision(2) << ind.rsi;
    if      (ind.rsi < 30) log << "  <- OVERSOLD";
    else if (ind.rsi > 70) log << "  <- OVERBOUGHT";
    log << "\n" << std::setprecision(4);
    log << "  MACD     : " << std::setw(10) << ind.macd
        << "  Signal: " << ind.macd_signal
        << "  Hist: " << ind.macd_hist << "\n";
    log << "  VWAP     : " << std::setw(10) << ind.vwap << "\n";
    log << "  ATR(14)  : " << std::setw(10) << ind.atr << "\n\n";

    log << "  SIGNAL\n";
    log << "  Score    :  " << (advice.score >= 0 ? "+" : "") << advice.score << " / 11\n";
    log << "  Decision :  " << advice.label << "\n";

    if (advice.signal == Signal::BUY || advice.signal == Signal::STRONG_BUY) {
        log << "\n  RISK MANAGEMENT\n";
        log << "  Entry       : " << c.close << "\n";
        log << "  Stop Loss   : " << advice.stop_loss   << "  (1.5x ATR below)\n";
        log << "  Take Profit : " << advice.take_profit << "  (3.0x ATR above)\n";
        log << "  Risk/Reward : " << std::setprecision(2) << advice.risk_reward << "x\n";
    }

    if (!ind.valid)
        log << "\n  [WARMING UP — need 30+ candles for reliable signals]\n";

    log << "\n";
    log.flush(); // write to disk immediately, don't wait
}

void printDashboard(const Candle& c, const Indicators& ind, const TradeAdvice& advice) {
    const std::string RESET = "\033[0m", BOLD = "\033[1m",
                      CYAN  = "\033[36m", DIM  = "\033[2m";

    std::cout << "\033[2J\033[H";
    std::cout << BOLD << CYAN;
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║        ALGO TRADING ENGINE  —  LIVE SIGNAL       ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n";
    std::cout << RESET;

    std::cout << DIM << "  Last candle : " << RESET << c.time << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << DIM << "  O:" << RESET << c.open
              << DIM << "  H:" << RESET << c.high
              << DIM << "  L:" << RESET << c.low
              << DIM << "  C:" << RESET << BOLD << c.close << RESET
              << DIM << "  Vol:" << RESET << (long long)c.volume << "\n\n";

    std::cout << BOLD << "  INDICATORS\n" << RESET;
    std::cout << "  SMA(20)      : " << std::setw(10) << ind.sma << "\n";
    std::cout << "  EMA(20)      : " << std::setw(10) << ind.ema << "\n";
    std::cout << "  RSI(14)      : " << std::setw(10) << std::setprecision(2) << ind.rsi;
    if      (ind.rsi < 30) std::cout << "  \033[32m← OVERSOLD\033[0m";
    else if (ind.rsi > 70) std::cout << "  \033[31m← OVERBOUGHT\033[0m";
    std::cout << "\n" << std::setprecision(4);
    std::cout << "  MACD         : " << std::setw(10) << ind.macd
              << "  Signal: " << ind.macd_signal
              << "  Hist: " << ind.macd_hist << "\n";
    std::cout << "  VWAP         : " << std::setw(10) << ind.vwap << "\n";
    std::cout << "  ATR(14)      : " << std::setw(10) << ind.atr << "\n\n";

    std::string color = signalColor(advice.signal);
    std::cout << BOLD << "  SIGNAL\n" << RESET;
    std::cout << "  Score        :  " << (advice.score >= 0 ? "+" : "") << advice.score << " / 11\n";
    std::cout << "  Decision     :  " << color << BOLD << advice.label << RESET << "\n\n";

    if (advice.signal == Signal::BUY || advice.signal == Signal::STRONG_BUY) {
        std::cout << BOLD << "  RISK MANAGEMENT  (if you enter here)\n" << RESET;
        std::cout << "  Entry        : " << std::setw(10) << c.close << "\n";
        std::cout << "  Stop Loss    : " << std::setw(10) << advice.stop_loss
                  << "  \033[31m▼ (1.5x ATR below)\033[0m\n";
        std::cout << "  Take Profit  : " << std::setw(10) << advice.take_profit
                  << "  \033[32m▲ (3.0x ATR above)\033[0m\n";
        std::cout << "  Risk/Reward  : " << std::setw(10) << std::setprecision(2)
                  << advice.risk_reward << "x\n";
    }

    if (!ind.valid)
        std::cout << "\n  \033[33m⚠  Warming up — need 30+ candles for reliable signals.\033[0m\n";

    std::cout << "\n  " << DIM << "Logging to signals.log  |  Ctrl+C to stop" << RESET << "\n";
    std::cout.flush();
}

int main() {
    // Append mode — restarting the engine never wipes old signals
    std::ofstream log("signals.log", std::ios::app);
    if (!log.is_open()) {
        std::cerr << "[engine] ERROR: Could not open signals.log\n";
        return 1;
    }

    // Session header so you can find where each run starts in the log
    std::time_t now = std::time(nullptr);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    log << "\n################################################\n";
    log << "  SESSION STARTED: " << timebuf << "\n";
    log << "################################################\n\n";
    log.flush();

    std::vector<Candle> candles;
    candles.reserve(500);
    const int MAX_WINDOW = 200;

    std::string line;
    while (std::getline(std::cin, line)) {
        Candle c;
        if (!parseCandle(line, c)) continue;

        candles.push_back(c);
        if ((int)candles.size() > MAX_WINDOW)
            candles.erase(candles.begin());

        Indicators  ind    = computeAll(candles);
        TradeAdvice advice = evaluate(c, ind);

        printDashboard(c, ind, advice); // live terminal (colors)
        writeLog(log, c, ind, advice);  // signals.log (plain text, appended)
    }

    return 0;
}