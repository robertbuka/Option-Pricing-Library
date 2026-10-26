# Option Pricing Library v1.0

Object-oriented C++ library that implements generalized Black-Scholes-Merton model, which covers non-dividend stocks, continuous dividend yields, and futures, for European and perpetual American options, as well as a binomial engine for finite American options.

## Features

- European options: Black–Scholes-Merton prices and analytic sensitivities (delta, gamma, vega, theta), generalised with cost of carry b
- American perpetual options: closed-form prices, exercise boundaries and analytic sensitivities
- American options: binomial lattice (Cox-Ross-Rubinstein and Jarrow-Rudd), with delta and gamma read off the tree and vega and theta by divided differences
- Grids: price or a sensitivity over a range of one parameter (Mesher), or a table over two (MatrixTable)
- Matrices: MatrixPricer prices any number of options at once and handles CSV files
- Additional features: binomial engine class template for appropriate option types, divided-difference delta and gamma, put–call parity for European options

## Validation

- European formulas for price and sensitivities match QuantLib within 3 × 10⁻¹³ [180 options].
- American binomial tree (CRR) matches QuantLib’s QD+ fixed-point engine within 7.9 × 10⁻⁴ (call) and 7.7 × 10⁻⁴ (put) [180 options, 10,000 steps], and within 8.0 × 10⁻³ (call) and 7.2 × 10⁻³ (put) across 10,000 random options at about 0.44 ms per option on average [1,000 steps, Apple M4].
- European binomial tree (CRR and JR) converges to Black–Scholes at first order: a 10× increase in steps cuts the error 10× (worst error at 10,000 steps: 7.9 × 10⁻⁴ CRR, 1.3 × 10⁻³ JR) [180 options].

Full results, references and how to reproduce them: [VALIDATION.md](VALIDATION.md).

## Example

```cpp
// include headers and set namespace
#include "EuropeanOption.hpp"
#include "AmericanOption.hpp"

#include <vector>
#include <iostream>

using namespace option_pricing_lib;

// create vector of inputs
// T, K, sig, r, S, b
std::vector<double> inputs = {1.0, 10, 0.5, 0.12, 5, 0.12};

// create two option objects
EuropeanOption eu_option(inputs);
AmericanOption am_option(inputs);

// get call price and delta for both
// AmericanOption uses binomial engine by default
std::cout << eu_option.CallPrice() << std::endl;
std::cout << eu_option.CallDelta() << std::endl;

std::cout << am_option.CallPrice() << std::endl;
std::cout << am_option.CallDelta() << std::endl;
```

## Requirements

- A C++20 compiler
- CMake 3.20 or newer
- Boost, headers only:
  - macOS: `sudo port install boost` (MacPorts) or `brew install boost` (Homebrew)
  - Ubuntu/Debian: `sudo apt install libboost-dev`

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/test          # test suite: prints PASS/FAIL per check and a summary
./build/validation    # accuracy and speed results in VALIDATION.md
```

Run both from the repository root: `validation` reads its reference data from `data/`.
If CMake can't find Boost, tell it where Boost is installed.

## Project layout

```
option_pricing_lib/   library sources and test suite
validation/           validation.cpp: accuracy and speed against QuantLib and Black–Scholes
data/                 QuantLib reference data read by validation.cpp
scripts/              make_reference_data.py: regenerates data/
VALIDATION.md         validation results
```

## Reference data

The files in `data/` are produced by `scripts/make_reference_data.py` with QuantLib for Python (version 1.44). Python is only needed to regenerate results.

```sh
pip install QuantLib
python scripts/make_reference_data.py
```

## Licence

MIT. See [LICENSE](LICENSE.txt).
