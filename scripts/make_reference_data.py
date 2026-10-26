"""
make_reference_data.py

Writes QuantLib reference prices for validation.cpp into the data/ folder.
Row n of a reference file belongs to row n of its inputs file.
Every option is priced both as a call and as a put.

Conventions match the C++ library: T = days / 365, b = r - q
"""

import csv
import random
from pathlib import Path

import QuantLib as ql


# ---------------------------------------------------------------- settings

SPOT = 100.0
TODAY = ql.Date(2, ql.January, 2026)  # any fixed date works

# grid: every combination of these (5 x 4 x 3 x 3 = 180 options)
GRID_STRIKES = [80.0, 90.0, 100.0, 110.0, 120.0]
GRID_DAYS = [30, 182, 365, 1095]                        # about 1 month, 6 months, 1 year, 3 years
GRID_VOLS = [0.15, 0.30, 0.50]
GRID_RATES = [(0.05, 0.00), (0.05, 0.03), (0.02, 0.06)]  # (r, q): no dividends, dividends, dividends above r

# random set: drawn uniformly inside the same ranges as the grid
RANDOM_COUNT = 10_000
RANDOM_SEED = 2026


DATA_FOLDER = Path(__file__).resolve().parent.parent / "data"

DAY_COUNT = ql.Actual365Fixed()
CALENDAR = ql.NullCalendar()


# ---------------------------------------------------------------- inputs

def grid_rows():
    """Every combination of the grid values (days, K, sig, r, q)"""
    rows = []
    for strike in GRID_STRIKES:
        for days in GRID_DAYS:
            for vol in GRID_VOLS:
                for rate, dividend in GRID_RATES:
                    rows.append((days, strike, vol, rate, dividend))
    return rows

def random_rows():
    """RANDOM_COUNT options with a fixed seed"""
    generator = random.Random(RANDOM_SEED)
    rows = []
    for _ in range(RANDOM_COUNT):
        days = generator.randint(30, 1095)
        strike = round(generator.uniform(80.0, 120.0), 2)
        vol = round(generator.uniform(0.15, 0.50), 4)
        rate = round(generator.uniform(0.00, 0.05), 4)
        dividend = round(generator.uniform(0.00, 0.06), 4)
        rows.append((days, strike, vol, rate, dividend))
    return rows

def library_inputs(row):
    """One row in the C++ library format: T, K, sig, r, S, b"""
    days, strike, vol, rate, dividend = row
    maturity = days / 365.0
    carry = round(rate - dividend, 10)  # rounding to remove float noise
    return [maturity, strike, vol, rate, SPOT, carry]


# ---------------------------------------------------------------- QuantLib pricing

def market(vol, rate, dividend):
    """Flat Black-Scholes-Merton market for one option"""
    spot = ql.QuoteHandle(ql.SimpleQuote(SPOT))
    rate_curve = ql.YieldTermStructureHandle(ql.FlatForward(TODAY, rate, DAY_COUNT))
    dividend_curve = ql.YieldTermStructureHandle(ql.FlatForward(TODAY, dividend, DAY_COUNT))
    vol_surface = ql.BlackVolTermStructureHandle(ql.BlackConstantVol(TODAY, CALENDAR, vol, DAY_COUNT))
    return ql.BlackScholesMertonProcess(spot, dividend_curve, rate_curve, vol_surface)


def european_values(row, option_type):
    """QuantLib analytic European price, delta, gamma, vega, theta."""
    days, strike, vol, rate, dividend = row
    option = ql.VanillaOption(ql.PlainVanillaPayoff(option_type, strike),
                              ql.EuropeanExercise(TODAY + days))
    option.setPricingEngine(ql.AnalyticEuropeanEngine(market(vol, rate, dividend)))
    return [option.NPV(), option.delta(), option.gamma(), option.vega(), option.theta()]


def american_price(row, option_type, scheme):
    """QuantLib QD+ fixed-point American price.

    scheme picks the accuracy: high precision for the 180 grid options,
    accurate (much faster) for the 10,000 random ones
    """
    days, strike, vol, rate, dividend = row
    option = ql.VanillaOption(ql.PlainVanillaPayoff(option_type, strike),
                              ql.AmericanExercise(TODAY, TODAY + days))
    option.setPricingEngine(ql.QdFpAmericanEngine(market(vol, rate, dividend), scheme))
    return option.NPV()


# ---------------------------------------------------------------- writing

def write_csv(file_name, header, rows):
    path = DATA_FOLDER / file_name
    with open(path, "w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(header)
        for row in rows:
            writer.writerow([repr(float(value)) for value in row])  # repr keeps every digit
    print(f"wrote {file_name} ({len(rows)} rows)")


INPUT_HEADER = ["T", "K", "sig", "r", "S", "b"]
GREEKS = ["price", "delta", "gamma", "vega", "theta"]
EUROPEAN_HEADER = [f"call_{name}" for name in GREEKS] + [f"put_{name}" for name in GREEKS]
AMERICAN_HEADER = ["call_price", "put_price"]


# ---------------------------------------------------------------- main

def main():
    ql.Settings.instance().evaluationDate = TODAY
    DATA_FOLDER.mkdir(exist_ok=True)

    grid = grid_rows()
    randoms = random_rows()

    # grid: inputs, European reference, American reference
    write_csv("grid_inputs.csv", INPUT_HEADER, [library_inputs(row) for row in grid])
    write_csv("grid_european_reference.csv", EUROPEAN_HEADER,
              [european_values(row, ql.Option.Call) + european_values(row, ql.Option.Put) for row in grid])
    precise = ql.QdFpAmericanEngine.highPrecisionScheme()
    write_csv("grid_american_reference.csv", AMERICAN_HEADER,
              [[american_price(row, ql.Option.Call, precise), american_price(row, ql.Option.Put, precise)] for row in grid])

    # random set: inputs, American reference
    fast = ql.QdFpAmericanEngine.accurateScheme()
    write_csv("random_inputs.csv", INPUT_HEADER, [library_inputs(row) for row in randoms])
    write_csv("random_american_reference.csv", AMERICAN_HEADER,
              [[american_price(row, ql.Option.Call, fast), american_price(row, ql.Option.Put, fast)] for row in randoms])


if __name__ == "__main__":
    main()
