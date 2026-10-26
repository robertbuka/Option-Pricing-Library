//
//  validation.cpp
//  option_pricing_lib
//
//  validation of functionality against Quantlib (Python v1.44) and Black-Scholes

#include "Option.hpp"
#include "AmericanPerpetualOption.hpp"
#include "EuropeanOption.hpp"
#include "MatrixPricer.hpp"
#include "GridUtilities.hpp"
#include "BinomialEngine.hpp"
#include "BinomialStrategy.hpp"
#include "AmericanOption.hpp"
#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <chrono>

using namespace option_pricing_lib;

// forward declarations
void printHeader();
void studyEuropeanFormulas();
void studyTreeConvergence();
void studyTreeGreeks();
void studyAmericanVsQuantLib();
void studyAmericanProperties();
void studyPerpetualLimit();
void studyAmericanVegaTheta();

int main()
{
    try
    {
        printHeader();
        studyEuropeanFormulas();
        studyTreeConvergence();
        studyTreeGreeks();
        studyAmericanVsQuantLib();
        studyAmericanProperties();
        studyPerpetualLimit();
        studyAmericanVegaTheta();
    }
    catch (const std::exception& error)
    {
        std::cout << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "unhandled exception" << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

// result of comparing computed values with reference values
struct ComparisonResult
{
    double absolute_error_largest;          // largest |computed - reference|
    std::size_t worst_row;                         // row where the largest error happened (0 = first data row)
    double absolute_error_average;          // average |computed - reference|
    double absolute_error_99th_percentile;  // 99% of rows have an error at or below this
    std::size_t rows_compared;                     // number of rows compared
};

// helpers

std::vector<double> readReferenceColumn(const std::string& file_path, const std::string& column_name)
{// read one named column of a reference csv
    
    std::ifstream csv_file(file_path);
    
    // check for valid path
    if (!csv_file.is_open()) throw std::invalid_argument("readReferenceColumn: invalid file path: " + file_path);
    
    // get all lines from csv
    std::vector<std::string> all_lines_vector;
    for (std::string line; std::getline(csv_file, line);)
    {
        all_lines_vector.push_back(line);
    }
    
    // check for empty file
    if (all_lines_vector.empty()) throw std::invalid_argument("readReferenceColumn: input file is empty");
    
    // extract and trim header
    boost::algorithm::trim(all_lines_vector[0]);
    std::stringstream first_line;
    first_line << all_lines_vector[0];
    std::vector<std::string> header;
    
    for (std::string column; std::getline(first_line, column, ',');)
    {
        boost::algorithm::trim(column);
        header.push_back(column);
    }
    
    // find index of input column
    
    auto column_iterator = std::find(header.begin(), header.end(), column_name);
    if (column_iterator == header.end()) throw std::invalid_argument("readReferenceColumn: invalid column name '" + column_name + "'");
    std::size_t column_index = std::distance(header.begin(), column_iterator);
    
    // extract numerical data
    std::vector<std::vector<double>> data;
    
    for (auto it = all_lines_vector.begin() + 1; it != all_lines_vector.end(); ++it)
    {
        // check for empty line
        boost::algorithm::trim(*it);
        if (*it == "") continue;
        
        std::vector<double> row;
        std::stringstream line;
        line << *it;
        
        // convert strings to doubles
        int column_counter = 1;
        for (std::string value; std::getline(line, value, ',');)
        {
            try
            {
                std::size_t elements_used;
                boost::algorithm::trim(value);
                double d_value = std::stod(value, &elements_used);
                
                if (elements_used != value.length()) throw std::exception();
                
                row.push_back(d_value);
            }
            catch (const std::exception&)
            {
                throw std::invalid_argument("readReferenceColumn: invalid value row " + std::to_string(it - all_lines_vector.begin() + 1) + ", column " + std::to_string(column_counter) + " '" + value + "'");
            }
            
            column_counter++;
        }
        // check that the row reaches the requested column
        if (row.size() <= column_index) throw std::invalid_argument("readReferenceColumn: row " + std::to_string(it - all_lines_vector.begin() + 1) + " has " + std::to_string(row.size()) + " values, needs at least " + std::to_string(column_index + 1));
        
        data.push_back(row);
    }
    
    // extract input column
    
    std::vector<double> output;
    output.reserve(data.size());
    
    for (const auto& element: data)
    {
        output.push_back(element[column_index]);
    }
    
    if (output.empty()) throw std::invalid_argument("readReferenceColumn: input column is empty");
    
    return output;
}

ComparisonResult compareValues(const std::vector<double>& computed_values, const std::vector<double>& reference_values)
{// compare two vectors of equal length
    
    // check for matching sizes
    if (computed_values.size() != reference_values.size()) throw std::invalid_argument("compareValues: vectors have different sizes (" + std::to_string(computed_values.size()) + " and " + std::to_string(reference_values.size()) + ")");
    if (computed_values.empty()) throw std::invalid_argument("compareValues: vectors are empty");
    
    // absolute error of every row
    std::vector<double> errors;
    errors.reserve(computed_values.size());
    
    for (std::size_t row_index = 0; row_index < computed_values.size(); ++row_index)
    {
        double error = std::abs(computed_values[row_index] - reference_values[row_index]);
        
        // check for a NaN
        if (std::isnan(error)) throw std::invalid_argument("compareValues: NaN in row " + std::to_string(row_index));
        
        errors.push_back(error);
    }
    
    ComparisonResult result;
    result.rows_compared = errors.size();
    
    // largest error and the row it happened in
    auto largest_iterator = std::max_element(errors.begin(), errors.end());
    result.absolute_error_largest = *largest_iterator;
    result.worst_row = std::distance(errors.begin(), largest_iterator);
    
    // average error
    result.absolute_error_average = std::accumulate(errors.begin(), errors.end(), 0.0) / errors.size();
    
    // 99th percentile: sort a copy (the original order gives the worst row), take the value 99% of the way up
    std::vector<double> sorted_errors(errors);
    std::sort(sorted_errors.begin(), sorted_errors.end());
    std::size_t percentile_index = std::ceil(0.99 * sorted_errors.size()) - 1;
    result.absolute_error_99th_percentile = sorted_errors[percentile_index];
    
    return result;
}

void printHeader()
{// print the title of the run and build (release/debug)
    std::cout << "=== option pricing library: validation run ===" << std::endl;
    
    // timings are only meaningful in an optimised (Release) build
#ifdef __OPTIMIZE__
    std::cout << "build: Release" << std::endl;
#else
    std::cout << "build: Debug, timings are not meaningful" << std::endl;
#endif
}

void printTitle(const std::string& title)
{// print the title line of a study
    std::cout << std::endl << "--- " << title << " ---" << std::endl;
}

void printResult(const std::string& label, const ComparisonResult& result)
{// print one comparison as a line
    std::cout << label << ": largest " << result.absolute_error_largest << " (row " << result.worst_row << ")"
              << ", average " << result.absolute_error_average
              << ", 99% " << result.absolute_error_99th_percentile
              << ", rows " << result.rows_compared << std::endl;
}

// studies (sections of the validation.md)

void studyEuropeanFormulas()
{// A: European prices and greeks vs QuantLib
    
    std::string input_path = "data/grid_inputs.csv";
    std::string reference_path = "data/grid_european_reference.csv";
    
    MatrixPricer<EuropeanOption> eu_option = MatrixPricer<EuropeanOption>::ReadCSV(input_path);
    
    // call
    printTitle("European call/put");
    
    ComparisonResult result = compareValues(MatrixPrice(eu_option, OptionTypeParameter::Call), readReferenceColumn(reference_path, "call_price"));
    printResult("call price", result);
    
    result = compareValues(MatrixDelta(eu_option, OptionTypeParameter::Call), readReferenceColumn(reference_path, "call_delta"));
    printResult("call delta", result);
    
    result = compareValues(MatrixGamma(eu_option, OptionTypeParameter::Call), readReferenceColumn(reference_path, "call_gamma"));
    printResult("call gamma", result);
    
    result = compareValues(MatrixVega(eu_option, OptionTypeParameter::Call), readReferenceColumn(reference_path, "call_vega"));
    printResult("call vega", result);
    
    result = compareValues(MatrixTheta(eu_option, OptionTypeParameter::Call), readReferenceColumn(reference_path, "call_theta"));
    printResult("call theta", result);
    // put
    result = compareValues(MatrixPrice(eu_option, OptionTypeParameter::Put), readReferenceColumn(reference_path, "put_price"));
    printResult("put price", result);
    
    result = compareValues(MatrixDelta(eu_option, OptionTypeParameter::Put), readReferenceColumn(reference_path, "put_delta"));
    printResult("put delta", result);
    
    result = compareValues(MatrixGamma(eu_option, OptionTypeParameter::Put), readReferenceColumn(reference_path, "put_gamma"));
    printResult("put gamma", result);
    
    result = compareValues(MatrixVega(eu_option, OptionTypeParameter::Put), readReferenceColumn(reference_path, "put_vega"));
    printResult("put vega", result);
    
    result = compareValues(MatrixTheta(eu_option, OptionTypeParameter::Put), readReferenceColumn(reference_path, "put_theta"));
    printResult("put theta", result);
    
}

void studyTreeConvergence()
{// B: tree price vs Black-Scholes as steps grow, with runtimes
    
    printTitle("European tree, price convergence");
    
    std::string input_path = "data/grid_inputs.csv";
    
    std::vector<long> b1_step_count = {10,100,1000,10000};
    std::vector<long> b2_step_count = {10,11,50,51,100,101,500,501,1000,1001,5000,5001};
    
    MatrixPricer<EuropeanOption> grid = MatrixPricer<EuropeanOption>::ReadCSV(input_path);
    
    // CRR
    for (const auto& steps: b1_step_count)
    {
        std::vector<double> tree_prices = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), CRR(), steps);
            return engine.Price(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_prices = MatrixHelper(grid, [](const Option& option)
        {
            return option.CallPrice();
        });
        
        ComparisonResult result = compareValues(tree_prices, black_scholes_prices);
        printResult("european call grid (CRR), " + std::to_string(steps) + " steps", result);
    }
    
    // JR
    for (const auto& steps: b1_step_count)
    {
        std::vector<double> tree_prices = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), JR(), steps);
            return engine.Price(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_prices = MatrixHelper(grid, [](const Option& option)
        {
            return option.CallPrice();
        });
        
        ComparisonResult result = compareValues(tree_prices, black_scholes_prices);
        printResult("european call grid (JR), " + std::to_string(steps) + " steps", result);
    }
    
    std::cout << "european call tree convergence (CRR, 10 to 5001 steps): " << std::endl;
    
    // one at-the-money option in detail
    // T, K, sig, r, S, b
    
    EuropeanOption eu_atm(1.0, 100, 0.3, 0.05, 100, 0.05);
    double bs_price = eu_atm.CallPrice();
    
    for (const auto& steps: b2_step_count)
    {
        BinomialEngine<EuropeanOption> eu_atm_bin(eu_atm, CRR(), steps);
        double tree_price = eu_atm_bin.Price(OptionTypeParameter::Call);
        
        // error against Black-Scholes
        double error = tree_price - bs_price;
        
        std::cout << steps << " steps, " << tree_price << " tree price, " << error << " signed error, " << error * steps << " error * steps" << std::endl;
    }
    
}

void studyTreeGreeks()
{// C: tree delta and gamma vs analytic
    
    printTitle("European tree, delta and gamma convergence");
    
    std::string input_path = "data/grid_inputs.csv";
    
    std::vector<long> step_count = {10,11,50,51,100,101,500,501,1000,1001,5000,5001};
    
    MatrixPricer<EuropeanOption> grid = MatrixPricer<EuropeanOption>::ReadCSV(input_path);
    
    // CRR
    std::cout << "(CRR, 10 to 5001 steps): " << std::endl;
    for (const auto& steps: step_count)
    {
        std::vector<double> tree_delta = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), CRR(), steps);
            return engine.Delta(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_delta = MatrixDelta(grid, OptionTypeParameter::Call);
        
        ComparisonResult result = compareValues(tree_delta, black_scholes_delta);
        printResult("european call delta, " + std::to_string(steps) + " steps", result);
    }
    
    std::cout << "(CRR, 10 to 5001 steps): " << std::endl;
    for (const auto& steps: step_count)
    {
        std::vector<double> tree_gamma = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), CRR(), steps);
            return engine.Gamma(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_gamma = MatrixGamma(grid, OptionTypeParameter::Call);
        
        ComparisonResult result = compareValues(tree_gamma, black_scholes_gamma);
        printResult("european call gamma, " + std::to_string(steps) + " steps", result);
    }
    
    // JR
    std::cout << "(JR, 10 to 5001 steps): " << std::endl;
    for (const auto& steps: step_count)
    {
        std::vector<double> tree_delta = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), JR(), steps);
            return engine.Delta(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_delta = MatrixDelta(grid, OptionTypeParameter::Call);
        
        ComparisonResult result = compareValues(tree_delta, black_scholes_delta);
        printResult("european call delta, " + std::to_string(steps) + " steps", result);
    }
    
    std::cout << "(JR, 10 to 5001 steps): " << std::endl;
    for (const auto& steps: step_count)
    {
        std::vector<double> tree_gamma = MatrixHelper(grid, [steps](const Option& option)
        {
            BinomialEngine<EuropeanOption> engine(static_cast<const EuropeanOption&>(option), JR(), steps);
            return engine.Gamma(OptionTypeParameter::Call);
        });
        
        std::vector<double> black_scholes_gamma = MatrixGamma(grid, OptionTypeParameter::Call);
        
        ComparisonResult result = compareValues(tree_gamma, black_scholes_gamma);
        printResult("european call gamma, " + std::to_string(steps) + " steps", result);
    }
    
}

void studyAmericanVsQuantLib()
{// D: grid and random set vs QuantLib
    printTitle("American tree (CRR), price against QD+ fixed point engine");
    
    std::string grid_input_path = "data/grid_inputs.csv";
    std::string random_input_path = "data/random_inputs.csv";
    std::string grid_reference_path = "data/grid_american_reference.csv";
    std::string random_reference_path = "data/random_american_reference.csv";
    
    MatrixPricer<AmericanOption> am_option = MatrixPricer<AmericanOption>::ReadCSV(grid_input_path);
    
    // call/put, 1000 steps
    
    ComparisonResult result = compareValues(MatrixPrice(am_option, OptionTypeParameter::Call), readReferenceColumn(grid_reference_path, "call_price"));
    printResult("call price 1000 steps", result);
    
    result = compareValues(MatrixPrice(am_option, OptionTypeParameter::Put), readReferenceColumn(grid_reference_path, "put_price"));
    printResult("put price 1000 steps", result);
    
    // call/put, 10000 steps

    std::vector<double> prices = MatrixHelper(am_option, [](const Option& option){
        AmericanOption internal_option = static_cast<const AmericanOption&>(option);
        internal_option.Binomial_NSteps(10000);
        return internal_option.CallPrice();
    });
    
    result = compareValues(prices, readReferenceColumn(grid_reference_path, "call_price"));
    printResult("call price 10000 steps", result);
    
    prices = MatrixHelper(am_option, [](const Option& option){
        AmericanOption internal_option = static_cast<const AmericanOption&>(option);
        internal_option.Binomial_NSteps(10000);
        return internal_option.PutPrice();
    });
    
    result = compareValues(prices, readReferenceColumn(grid_reference_path, "put_price"));
    printResult("put price 10000 steps", result);
    
    // random grid
    
    am_option = MatrixPricer<AmericanOption>::ReadCSV(random_input_path);
    
    auto start = std::chrono::high_resolution_clock::now();
    result = compareValues(MatrixPrice(am_option, OptionTypeParameter::Call), readReferenceColumn(random_reference_path, "call_price"));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printResult("10k random set call", result);
    std::cout << "in " << duration << ", " << duration/10000.0 << " per option" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    result = compareValues(MatrixPrice(am_option, OptionTypeParameter::Put), readReferenceColumn(random_reference_path, "put_price"));
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printResult("10k random set put", result);
    std::cout << "in " << duration << ", " << duration/10000.0 << " per option" << std::endl;
    
}

void studyAmericanProperties()
{// E: no-arbitrage rules over a sweep
    
    printTitle("American properties");
    
    std::string grid_input_path = "data/grid_inputs.csv";
    std::string am_reference_path = "data/grid_american_reference.csv";
    std::string eu_reference_path = "data/grid_european_reference.csv";
    
    MatrixPricer<AmericanOption> am_option = MatrixPricer<AmericanOption>::ReadCSV(grid_input_path);
    MatrixPricer<EuropeanOption> eu_option = MatrixPricer<EuropeanOption>::ReadCSV(grid_input_path);
    
    std::vector<double> am_call_prices = MatrixPrice(am_option, OptionTypeParameter::Call);
    std::vector<double> am_put_prices = MatrixPrice(am_option, OptionTypeParameter::Put);
    // eu prices using binomial engine
    std::vector<double> eu_call_prices = MatrixHelper(eu_option, [](const Option& option){
        BinomialEngine<EuropeanOption> eu_bin(static_cast<const EuropeanOption&>(option), CRR(), 1000);
        return eu_bin.Price(OptionTypeParameter::Call);
    });
    std::vector<double> eu_put_prices = MatrixHelper(eu_option, [](const Option& option){
        BinomialEngine<EuropeanOption> eu_bin(static_cast<const EuropeanOption&>(option), CRR(), 1000);
        return eu_bin.Price(OptionTypeParameter::Put);
    });
    
    // American >= European
    
    // call
    int failed = 0;
    int checked = 0;
    
    for (std::size_t counter = 0; counter < am_call_prices.size(); ++counter)
    {
        if (am_call_prices[counter] < eu_call_prices[counter] - 1e-12) ++failed;
        ++checked;
    }
    std::cout << "American >= European (call): " << checked << " checked, " << failed << " failed" << std::endl;
    
    // put
    failed = 0;
    checked = 0;
    
    for (std::size_t counter = 0; counter < am_call_prices.size(); ++counter)
    {
        if (am_put_prices[counter] < eu_put_prices[counter] - 1e-12) ++failed;
        ++checked;
    }
    std::cout << "American >= European (put): " << checked << " checked, " << failed << " failed" << std::endl;
    
    // American >= intrinsic value
    
    // call
    std::vector<double> call_intrinsic = MatrixHelper(am_option, [](const Option& option)
    {
        return std::max(option.S() - option.K(), 0.0);
    });
    
    failed = 0;
    checked = 0;
    
    for (std::size_t counter = 0; counter < am_call_prices.size(); ++counter)
    {
        if (am_call_prices[counter] < call_intrinsic[counter] - 1e-12) ++failed;
        ++checked;
    }
    std::cout << "American >= intrinsic value (call): " << checked << " checked, " << failed << " failed" << std::endl;
    
    // put
    std::vector<double> put_intrinsic = MatrixHelper(am_option, [](const Option& option)
    {
        return std::max(option.K() - option.S(), 0.0);
    });
    
    failed = 0;
    checked = 0;
    
    for (std::size_t counter = 0; counter < am_put_prices.size(); ++counter)
    {
        if (am_put_prices[counter] < put_intrinsic[counter] - 1e-12) ++failed;
        ++checked;
    }
    std::cout << "American >= intrinsic value (put): " << checked << " checked, " << failed << " failed" << std::endl;
    
    // American call (b >= r) == European
    std::vector<double> carry_minus_rate = MatrixHelper(am_option, [](const Option& option)
    {
        return option.b() - option.r();
    });
    
    failed = 0;
    checked = 0;
    
    for (std::size_t counter = 0; counter < am_call_prices.size(); ++counter)
    {
        // skip rows with b < r, where early exercise can matter
        if (carry_minus_rate[counter] < 0) continue;
        
        if (std::abs(am_call_prices[counter] - eu_call_prices[counter]) > 1e-12) ++failed;
        ++checked;
    }
    std::cout << "American = European when b >= r (call): " << checked << " checked, " << failed << " failed" << std::endl;
    
}

void studyPerpetualLimit()
{// F: long-maturity American vs perpetual
    
    printTitle("Long-maturity American convergence to perpetual");
    
    std::vector<double> t_vector = {1, 2, 5, 10, 20, 30, 50, 100};
    
    AmericanPerpetualOption am_perp(100,0.1,0.1,110,0.02);
    AmericanOption am_option(1,100,0.1,0.1,110,0.02);
    
    // call
    double am_perp_price = am_perp.CallPrice();
    for (const auto& t: t_vector)
    {
        am_option.SetParameter(OptionParameter::T, t);
        am_option.Binomial_NSteps(t * 100);
        double am_price = am_option.CallPrice();
        std::cout << "CALL " << t << " year(s) maturity, " << am_price << " American price, " << am_perp_price << " perpetual price, " << am_perp_price - am_price << " gap" << std::endl;
    }
    
    // put
    am_perp_price = am_perp.PutPrice();
    for (const auto& t: t_vector)
    {
        am_option.SetParameter(OptionParameter::T, t);
        am_option.Binomial_NSteps(t * 100);
        double am_price = am_option.PutPrice();
        std::cout << "PUT " << t << " year(s) maturity, " << am_price << " American price, " << am_perp_price << " perpetual price, " << am_perp_price - am_price << " gap" << std::endl;
    }
    
}

void studyAmericanVegaTheta()
{// G: bump size vs error for American vega and theta
    
    printTitle("American vega and theta, bump size vs error");
    
    // at-the-money call with b = r
    // early exercise is never optimal
    // so Black-Scholes vega and theta are exact references
    AmericanOption am_atm(1.0, 100, 0.3, 0.05, 100, 0.05);
    EuropeanOption eu_atm(1.0, 100, 0.3, 0.05, 100, 0.05);
    
    double bs_vega = eu_atm.CallVega();
    double bs_theta = eu_atm.CallTheta();
    std::cout << "Black-Scholes vega " << bs_vega << ", theta " << bs_theta << std::endl;
    
    std::vector<double> bump_vector = {0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001};
    
    for (const auto& bump: bump_vector)
    {
        AmericanOption internal_option(am_atm);
        
        // divided differences vega
        internal_option.SetParameter(OptionParameter::sig, am_atm.sig() + bump);
        double price_plus_bump = internal_option.CallPrice();
        internal_option.SetParameter(OptionParameter::sig, am_atm.sig() - bump);
        double price_minus_bump = internal_option.CallPrice();
        double vega = (price_plus_bump - price_minus_bump) / (2 * bump);
        
        // divided differences theta
        internal_option.SetParameter(OptionParameter::sig, am_atm.sig());
        internal_option.SetParameter(OptionParameter::T, am_atm.T() + bump);
        price_plus_bump = internal_option.CallPrice();
        internal_option.SetParameter(OptionParameter::T, am_atm.T() - bump);
        price_minus_bump = internal_option.CallPrice();
        double theta = -(price_plus_bump - price_minus_bump) / (2 * bump);
        
        std::cout << "CALL bump " << bump << ": vega error " << vega - bs_vega << ", theta error " << theta - bs_theta << std::endl;
    }
}
