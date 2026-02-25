"""
main.py
-------
Entry point for the real-time trading signal engine.

What this does:
  1. Compiles the C++ engine (if not already built)
  2. Starts the C++ engine as a subprocess
  3. Every 60 seconds, fetches the latest candle from Yahoo Finance
  4. Pipes it to the C++ engine via stdin
  5. C++ computes indicators + signal and updates the terminal display

Usage:
    python main.py --symbol AAPL
    python main.py --symbol BTC-USD --interval 5m
"""

import subprocess
import sys
import os
import time
import argparse
import yfinance as yf
import pandas as pd
from datetime import datetime

# ─────────────────────────────────────────────
# Paths
# ─────────────────────────────────────────────
SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
ENGINE_DIR  = os.path.join(SCRIPT_DIR, "cpp_engine")
import platform
ENGINE_BIN  = os.path.join(ENGINE_DIR, "engine.exe" if platform.system() == "Windows" else "engine")

# ─────────────────────────────────────────────
# Build the C++ engine if needed
# ─────────────────────────────────────────────
def build_engine():
    sources = ["main.cpp", "indicators.cpp", "strategy.cpp"]
    src_paths = [os.path.join(ENGINE_DIR, s) for s in sources]
    out_path  = ENGINE_BIN

    # Check if any source is newer than the binary
    needs_build = not os.path.exists(out_path)
    if not needs_build:
        bin_mtime = os.path.getmtime(out_path)
        for src in src_paths:
            if os.path.getmtime(src) > bin_mtime:
                needs_build = True
                break

    if not needs_build:
        print("[build] Engine is up to date.")
        return

    print("[build] Compiling C++ engine...")
    cmd = ["g++", "-std=c++17", "-O2", "-o", out_path] + src_paths
    if platform.system() == "Windows":
        cmd += ["-lws2_32"]
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print("[build] ❌ Compilation failed:\n")
        print(result.stderr)
        sys.exit(1)

    print("[build] ✅ Compilation successful.\n")


# ─────────────────────────────────────────────
# Fetch the most recent completed candle
# ─────────────────────────────────────────────
def fetch_latest_candles(symbol: str, interval: str, warmup: bool = False) -> list[str]:
    """
    Returns a list of CSV lines (one per candle).
    On warmup=True, returns the last ~60 candles to seed the indicators.
    On warmup=False, returns just the most recent completed candle.
    """
    period = "7d" if interval == "1m" else "30d"

    try:
        ticker = yf.Ticker(symbol)
        df = ticker.history(interval=interval, period=period)
    except Exception as e:
        print(f"[fetcher] Error fetching data: {e}")
        return []

    if df.empty:
        return []

    # Drop the last row — it's the currently forming candle (incomplete)
    df = df.iloc[:-1]

    # Normalize
    df = df.reset_index()
    time_col = "Datetime" if "Datetime" in df.columns else "Date"
    df = df.rename(columns={time_col: "Time"})
    df["Time"] = pd.to_datetime(df["Time"]).dt.tz_localize(None)
    df["Time"] = df["Time"].dt.strftime("%Y-%m-%d %H:%M:%S")
    df = df[["Time", "Open", "High", "Low", "Close", "Volume"]].dropna()

    if warmup:
        # Send last 60 candles to warm up indicators
        df = df.tail(60)
    else:
        # Only send the latest completed candle
        df = df.tail(1)

    lines = []
    for _, row in df.iterrows():
        line = f"{row['Time']},{row['Open']:.4f},{row['High']:.4f},{row['Low']:.4f},{row['Close']:.4f},{int(row['Volume'])}"
        lines.append(line)

    return lines


# ─────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(description="Real-time algo trading signal engine.")
    parser.add_argument("--symbol",   type=str, default="AAPL",  help="Ticker symbol (default: AAPL)")
    parser.add_argument("--interval", type=str, default="1m",    help="Candle interval: 1m, 5m (default: 1m)")
    args = parser.parse_args()

    # Interval in seconds between fetches
    interval_seconds = {
        "1m":  60,
        "2m":  120,
        "5m":  300,
        "15m": 900,
        "1h":  3600,
    }.get(args.interval, 60)

    # Step 1: build the engine
    build_engine()

    # Step 2: start the C++ engine as a subprocess
    print(f"[main] Starting engine for {args.symbol} | interval={args.interval}")
    engine = subprocess.Popen(
        [ENGINE_BIN],
        stdin=subprocess.PIPE,
        text=True,
        bufsize=1  # line-buffered
    )

    def send(line: str):
        """Send a CSV candle line to the C++ engine."""
        try:
            engine.stdin.write(line + "\n")
            engine.stdin.flush()
        except BrokenPipeError:
            print("\n[main] Engine process died unexpectedly.")
            sys.exit(1)

    # Step 3: warm up — send historical candles so indicators are ready
    print("[main] Warming up indicators with historical candles...")
    warmup_lines = fetch_latest_candles(args.symbol, args.interval, warmup=True)
    for line in warmup_lines:
        send(line)
    print(f"[main] Sent {len(warmup_lines)} historical candles. Going live...\n")
    time.sleep(1) # small pause so the dashboard renders before we start the loop

    # Track the last candle time to avoid sending duplicates
    last_candle_time = warmup_lines[-1].split(",")[0] if warmup_lines else ""

    # Step 4: live loop
    try:
        while True:
            lines = fetch_latest_candles(args.symbol, args.interval, warmup=False)

            if lines:
                candle_time = lines[0].split(",")[0]
                if candle_time != last_candle_time:
                    send(lines[0])
                    last_candle_time = candle_time

            # Wait for the next candle
            time.sleep(interval_seconds)

    except KeyboardInterrupt:
        print("\n\n[main] Stopped by user.")
        engine.stdin.close()
        engine.wait()


if __name__ == "__main__":
    main()