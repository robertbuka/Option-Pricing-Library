//
//  test.cpp
//  option_pricing_lib
//
//  automated checks for the option pricing library.
//  every check prints one PASS/FAIL line
//  the program exits with code 1 if any check fails
//  or an unexpected exception is thrown, and with code 0 only if every check ran and passed.


#include "Option.hpp"
#include "AmericanPerpetualOption.hpp"
#include "EuropeanOption.hpp"
#include "MatrixPricer.hpp"
#include "GridUtilities.hpp"
#include "BinomialEngine.hpp"
#include "BinomialStrategy.hpp"
#include "AmericanOption.hpp"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <fstream>

int fail_counter = 0;
int checks_counter = 0;

// check computed value against an expected value
void checkValue(const std::string& name, double computed_value, double expected_value, double tolerance)
{
    std::string output;

    if (std::abs(computed_value - expected_value) <= tolerance)
    {
        output = "PASS";
    }
    else
    {
        output = "FAIL";
        fail_counter++;
    }

    checks_counter++;
    std::cout << std::setprecision(10) << output << " " << std::left << std::setw(20) << name << " " << std::fixed << computed_value << ", expected " << expected_value << ", error " << std::setprecision(3) << std::scientific  << computed_value - expected_value << ", tolerance " << tolerance << std::endl;
}

// check two vectors element by element
void checkVector(const std::string& name, const std::vector<double>& computed_values, const std::vector<double>& expected_values, double tolerance)
{
    checks_counter++;

    // different sizes or nothing to compare
    if (computed_values.size() != expected_values.size() || computed_values.empty())
    {
        fail_counter++;
        std::cout << "FAIL " << std::left << std::setw(20) << name << " size mismatch or empty: "
                  << computed_values.size() << " vs " << expected_values.size() << std::endl;
        return;
    }

    double max_error = 0.0;
    std::size_t max_err_index = 0;
    bool failed = false;

    for (std::size_t index = 0; index < computed_values.size(); ++index)
    {
        double error = std::abs(computed_values[index] - expected_values[index]);

        if (!(error <= tolerance)) failed = true;

        // track the largest error
        if (error > max_error || (std::isnan(error) && !std::isnan(max_error)))
        {
            max_error = error;
            max_err_index = index;
        }
    }

    if (failed) fail_counter++;

    std::cout << (failed ? "FAIL" : "PASS") << " " << std::left << std::setw(20) << name << " "
              << computed_values.size() << " points, largest error " << std::setprecision(3) << std::scientific
              << max_error << " at index " << max_err_index << ", tolerance " << tolerance << std::endl;
}

// check that running input function throws std::invalid_argument
void checkThrows(const std::string& name, const std::function<void()>& input_func)
{
    checks_counter++;

    std::string result;
    bool passed = false;

    try
    {
        input_func();
        result = "no exception thrown";
    }
    catch (const std::invalid_argument&)
    {
        passed = true;
        result = "threw invalid_argument";
    }
    catch (const std::exception& error)
    {
        result = std::string("wrong exception type: ") + error.what();
    }
    catch (...)
    {
        result = "unknown exception";
    }

    if (!passed) fail_counter++;

    std::cout << (passed ? "PASS" : "FAIL") << " " << std::left << std::setw(20) << name << " " << result << std::endl;
}

// apply a function to every point of a mesh
template <typename Function>
std::vector<double> applyToEach(const std::vector<double>& grid, Function function)
{
    std::vector<double> output;
    std::transform(grid.begin(), grid.end(), std::back_inserter(output), function);
    return output;
}

// write text into a file
void writeFile(const std::string& file_path, const std::string& content)
{
    std::ofstream file(file_path);
    file << content;
}

int main()
{
    using namespace option_pricing_lib;
    
    try
    {
        std::cout << std::fixed << std::setprecision(10); // increase default precision for better output

        // test pricers
        std::cout << "--- pricers ---" << std::endl;

        EuropeanOption batch_1(0.25,65,0.3,0.08,60,0.08);
        EuropeanOption batch_2(1.0, 100, 0.2, 0.0, 100, 0.0);
        EuropeanOption batch_3(1.0, 10, 0.5, 0.12, 5, 0.12);
        EuropeanOption batch_4(30, 100, 0.3, 0.08, 100, 0.08);
        AmericanPerpetualOption batch_a(100,0.1,0.1,110,0.02);

        // several batches from QuantLib testValues
        EuropeanOption batch_ql_1(0.5, 95, 0.20, 0.10, 100, 0.05);
        EuropeanOption batch_ql_2(0.75,19,0.28,0.10,19,0);
        EuropeanOption batch_ql_3(0.5,1.60,0.12,0.06,1.56,-0.02);
        EuropeanOption batch_ql_4(0.5,70,0.35,0.10,75,0.05);


        checkValue("batch 1 call", batch_1.CallPrice(), 2.13337, 5e-6);
        checkValue("batch 1 put", batch_1.PutPrice(), 5.84628, 5e-6);
        checkValue("batch 2 call", batch_2.CallPrice(), 7.96557, 5e-6);
        checkValue("batch 2 put", batch_2.PutPrice(), 7.96557, 5e-6);
        checkValue("batch 3 call", batch_3.CallPrice(), 0.204058, 5e-7);
        checkValue("batch 3 put", batch_3.PutPrice(), 4.07326, 5e-6);
        checkValue("batch 4 call", batch_4.CallPrice(), 92.1757, 5e-5);
        checkValue("batch 4 put", batch_4.PutPrice(), 1.2475, 5e-5);
        checkValue("batch a call", batch_a.CallPrice(), 18.5035, 5e-5);
        checkValue("batch a put", batch_a.PutPrice(), 3.0311, 5e-5);
        checkValue("batch 1ql put", batch_ql_1.PutPrice(), 2.4648, 5e-5);
        checkValue("batch 2ql put", batch_ql_2.PutPrice(), 1.7011, 5e-5);
        checkValue("batch 3ql call", batch_ql_3.CallPrice(), 0.0291, 5e-5);
        checkValue("batch 4ql put", batch_ql_4.PutPrice(), 4.0870, 5e-5);

        // binomial tree against black-scholes, N = 1000
        BinomialEngine<EuropeanOption> tree_1(batch_1, CRR(), 1000);
        BinomialEngine<EuropeanOption> tree_2(batch_2, CRR(), 1000);
        BinomialEngine<EuropeanOption> tree_3(batch_3, CRR(), 1000);
        BinomialEngine<EuropeanOption> tree_4(batch_4, CRR(), 1000);
        checkValue("batch 1 CRR call", tree_1.Price(OptionTypeParameter::Call), batch_1.CallPrice(), 1e-3);
        checkValue("batch 1 CRR put", tree_1.Price(OptionTypeParameter::Put), batch_1.PutPrice(), 1e-3);
        checkValue("batch 2 CRR call", tree_2.Price(OptionTypeParameter::Call), batch_2.CallPrice(), 3e-3);
        checkValue("batch 2 CRR put", tree_2.Price(OptionTypeParameter::Put), batch_2.PutPrice(), 3e-3);
        checkValue("batch 3 CRR call", tree_3.Price(OptionTypeParameter::Call), batch_3.CallPrice(), 5e-4);
        checkValue("batch 3 CRR put", tree_3.Price(OptionTypeParameter::Put), batch_3.PutPrice(), 5e-4);
        checkValue("batch 4 CRR call", tree_4.Price(OptionTypeParameter::Call), batch_4.CallPrice(), 5e-3);
        checkValue("batch 4 CRR put", tree_4.Price(OptionTypeParameter::Put), batch_4.PutPrice(), 5e-3);

        BinomialEngine<EuropeanOption> tree_1_jr(batch_1, JR(), 1000);
        BinomialEngine<EuropeanOption> tree_2_jr(batch_2, JR(), 1000);
        checkValue("batch 1 JR call", tree_1_jr.Price(OptionTypeParameter::Call), batch_1.CallPrice(), 1e-3);
        checkValue("batch 1 JR put", tree_1_jr.Price(OptionTypeParameter::Put), batch_1.PutPrice(), 1e-3);
        checkValue("batch 2 JR call", tree_2_jr.Price(OptionTypeParameter::Call), batch_2.CallPrice(), 3e-3);
        checkValue("batch 2 JR put", tree_2_jr.Price(OptionTypeParameter::Put), batch_2.PutPrice(), 3e-3);

        // american options, N = 1000
        AmericanOption batch_1_am(0.25,65,0.3,0.08,60,0.08);
        AmericanOption deep_put_am(1.0, 100, 0.2, 0.1, 50, 0.1);

        // b = r: early exercise of a call is never optimal, so it equals the european tree
        checkValue("batch 1 am call", batch_1_am.CallPrice(), tree_1.Price(OptionTypeParameter::Call), 0);
        checkValue("batch 1 am put >= EU put", batch_1_am.PutPrice() >= tree_1.Price(OptionTypeParameter::Put) ? 1.0 : 0.0, 1, 0.5);
        checkValue("batch 1 am put >= intrinsic", batch_1_am.PutPrice() >= batch_1_am.K() - batch_1_am.S() ? 1.0 : 0.0, 1, 0.5);

        // deep in the money put is exercised immediately
        checkValue("deep am put", deep_put_am.PutPrice(), 50, 1e-12);

        // test put-call parity
        std::cout << "--- put-call parity ---" << std::endl;

        // test double PutCallParity(double value, OptionTypeParameter option_type) const
        checkValue("batch 1 p-c parity, call input", batch_1.PutCallParity(batch_1.CallPrice(), OptionTypeParameter::Call), batch_1.PutPrice(), 1e-12 * std::max(1.0, std::abs(batch_1.PutPrice())));
        checkValue("batch 1 p-c parity, put input", batch_1.PutCallParity(batch_1.PutPrice(), OptionTypeParameter::Put), batch_1.CallPrice(), 1e-12 * std::max(1.0, std::abs(batch_1.CallPrice())));

        // test bool PutCallParity(double value_1, double value_2, double tolerance) const
        checkValue("batch 1 bool p-c parity", batch_1.PutCallParity(batch_1.CallPrice(), batch_1.PutPrice()) ? 1.0 : 0.0, 1, 0.5);
        checkValue("batch 1 bool p-c parity invalid", batch_1.PutCallParity(batch_1.CallPrice(), batch_2.PutPrice()) ? 1.0 : 0.0, 0, 0.5);

        // test that tolerance scales with the prices
        EuropeanOption parity_large(1.0, 1e8, 0.3, 0.08, 1.1e8, 0.05);
        checkValue("parity K=1e8 correct", parity_large.PutCallParity(parity_large.CallPrice(), parity_large.PutPrice()) ? 1.0 : 0.0, 1, 0.5);
        EuropeanOption parity_small(1.0, 0.01, 0.3, 0.08, 0.011, 0.05);
        checkValue("parity K=0.01 off 1e-9", parity_small.PutCallParity(parity_small.CallPrice(), parity_small.PutPrice() + 1e-9) ? 1.0 : 0.0, 0, 0.5);

        // test custom tolerance
        checkValue("parity off 1e-6 default", batch_1.PutCallParity(batch_1.CallPrice(), batch_1.PutPrice() + 1e-6) ? 1.0 : 0.0, 0, 0.5);
        checkValue("parity off 1e-6, tol 1e-7", batch_1.PutCallParity(batch_1.CallPrice(), batch_1.PutPrice() + 1e-6, 1e-7) ? 1.0 : 0.0, 1, 0.5);

        // test sensitivities
        std::cout << "--- the greeks ---" << std::endl;

        EuropeanOption g_test(0.5,100,0.36,0.1,105,0);

        checkValue("batch g call delta", g_test.CallDelta(), 0.5946, 5e-5);
        checkValue("batch g put delta", g_test.PutDelta(), -0.3566, 5e-5);
        checkValue("batch g call/put gamma", g_test.PutGamma(), g_test.CallGamma(), 1e-8);
        checkValue("call d - put d = e^((b−r)T)", g_test.CallDelta() - g_test.PutDelta(), std::exp((g_test.b() - g_test.r())*g_test.T()), 1e-12);

        // perpetual greeks (analytic)
        checkValue("batch a call delta", batch_a.CallDelta(), 0.5411416778388267, 1e-10);
        checkValue("batch a put delta", batch_a.PutDelta(), -0.17130976189152414, 1e-10);
        checkValue("batch a call gamma", batch_a.CallGamma(), 0.010906418133212779, 1e-10);
        checkValue("batch a put gamma", batch_a.PutGamma(), 0.011239463049451674, 1e-10);

        // vega and theta
        EuropeanOption batch_w_carry(0.5,100,0.36,0.1,105,0.03);
        checkValue("carry call vega", batch_w_carry.CallVega(), 26.630445044532294, 1e-10);
        checkValue("carry call/put vega", batch_w_carry.PutVega(), batch_w_carry.CallVega(), 0);
        checkValue("carry call theta", batch_w_carry.CallTheta(), -10.216308345019073, 1e-10);
        checkValue("carry put theta", batch_w_carry.PutTheta(), -7.801213909505046, 1e-10);

        // perpetual vega and theta
        checkValue("batch a call vega", batch_a.CallVega(), 77.49246561193259, 1e-10);
        checkValue("batch a put vega", batch_a.PutVega(), 70.4819262483114, 1e-10);
        checkValue("batch a call theta", batch_a.CallTheta(), 0, 0);
        checkValue("batch a put theta", batch_a.PutTheta(), 0, 0);

        // binomial tree greeks against analytic, N = 1000
        BinomialEngine<EuropeanOption> tree_g(g_test, CRR(), 1000);
        BinomialEngine<EuropeanOption> tree_g_jr(g_test, JR(), 1000);
        checkValue("batch 1 CRR call delta", tree_1.Delta(OptionTypeParameter::Call), batch_1.CallDelta(), 1e-4);
        checkValue("batch 1 CRR put delta", tree_1.Delta(OptionTypeParameter::Put), batch_1.PutDelta(), 1e-4);
        checkValue("batch 1 CRR gamma", tree_1.Gamma(OptionTypeParameter::Call), batch_1.CallGamma(), 5e-5);
        checkValue("batch g CRR call delta", tree_g.Delta(OptionTypeParameter::Call), g_test.CallDelta(), 1e-4);
        checkValue("batch g CRR put delta", tree_g.Delta(OptionTypeParameter::Put), g_test.PutDelta(), 1e-4);
        checkValue("batch g CRR gamma", tree_g.Gamma(OptionTypeParameter::Call), g_test.CallGamma(), 5e-5);
        checkValue("batch g JR call delta", tree_g_jr.Delta(OptionTypeParameter::Call), g_test.CallDelta(), 1e-4);
        checkValue("batch g JR put delta", tree_g_jr.Delta(OptionTypeParameter::Put), g_test.PutDelta(), 1e-4);
        checkValue("batch g JR gamma", tree_g_jr.Gamma(OptionTypeParameter::Call), g_test.CallGamma(), 5e-5);

        // american call greeks, b = r: same as european
        checkValue("batch 1 am call delta", batch_1_am.CallDelta(), tree_1.Delta(OptionTypeParameter::Call), 0);
        checkValue("batch 1 am call gamma", batch_1_am.CallGamma(), tree_1.Gamma(OptionTypeParameter::Call), 0);
        checkValue("batch 1 am call vega", batch_1_am.CallVega(), batch_1.CallVega(), 5e-2);
        checkValue("batch 1 am call theta", batch_1_am.CallTheta(), batch_1.CallTheta(), 3e-2);

        // test divided differences method
        std::cout << "--- divided differences ---" << std::endl;

        checkValue("batch g DD call delta", g_test.DividedDifferencesCallDelta(1e-4), g_test.CallDelta(), 1e-8);
        checkValue("batch g DD put delta", g_test.DividedDifferencesPutDelta(1e-4), g_test.PutDelta(), 1e-8);
        checkValue("batch g DD gamma", g_test.DividedDifferencesCallGamma(1e-2), g_test.CallGamma(), 1e-8);

        // analytic perpetual greeks against divided differences
        checkValue("batch a DD call delta", batch_a.DividedDifferencesCallDelta(1e-4), batch_a.CallDelta(), 1e-8);
        checkValue("batch a DD put delta", batch_a.DividedDifferencesPutDelta(1e-4), batch_a.PutDelta(), 1e-8);
        checkValue("batch a DD call gamma", batch_a.DividedDifferencesCallGamma(1e-2), batch_a.CallGamma(), 1e-8);
        checkValue("batch a DD put gamma", batch_a.DividedDifferencesPutGamma(1e-2), batch_a.PutGamma(), 1e-8);

        // test getters, constructors, copying and assignment
        std::cout << "--- classes functionality ---" << std::endl;

        // default constructor + setters, then getters return the values set
        EuropeanOption via_setters;
        via_setters.T(0.25);
        via_setters.K(65);
        via_setters.sig(0.3);
        via_setters.r(0.08);
        via_setters.S(60);
        via_setters.b(0.08);
        checkVector("setters and getters", {via_setters.T(), via_setters.K(), via_setters.sig(), via_setters.r(), via_setters.S(), via_setters.b()}, {0.25, 65, 0.3, 0.08, 60, 0.08}, 0);

        // vector constructor: same parameters as batch 3, in the order T, K, sig, r, S, b
        EuropeanOption via_vector(std::vector<double>{1.0, 10, 0.5, 0.12, 5, 0.12});
        checkValue("vector ctor (EU)", via_vector.CallPrice(), batch_3.CallPrice(), 0);

        // copy constructor and assignment operator
        EuropeanOption eu_copy(batch_1);
        EuropeanOption eu_assigned;
        eu_assigned = batch_1;
        checkValue("copy ctor (EU)", eu_copy.CallPrice(), batch_1.CallPrice(), 0);
        checkValue("assignment (EU)", eu_assigned.PutPrice(), batch_1.PutPrice(), 0);

        AmericanPerpetualOption perp_vector(std::vector<double>{100, 0.1, 0.1, 110, 0.02});
        AmericanPerpetualOption perp_copy(batch_a);
        AmericanPerpetualOption perp_assigned;
        perp_assigned = batch_a;
        checkValue("vector ctor (perp)", perp_vector.CallPrice(), batch_a.CallPrice(), 0);
        checkValue("copy ctor (perp)", perp_copy.CallPrice(), batch_a.CallPrice(), 0);
        checkValue("assignment (perp)", perp_assigned.PutPrice(), batch_a.PutPrice(), 0);

        // clone: a full copy of the right type behind an Option pointer
        std::unique_ptr<Option> eu_clone = batch_1.clone();
        std::unique_ptr<Option> perp_clone = batch_a.clone();
        checkValue("clone (EU)", eu_clone->CallPrice(), batch_1.CallPrice(), 0);
        checkValue("clone (perp)", perp_clone->CallPrice(), batch_a.CallPrice(), 0);

        // get parameter returns the same values as the getters
        checkVector("get parameter (EU)", {batch_1.GetParameter(OptionParameter::T), batch_1.GetParameter(OptionParameter::K), batch_1.GetParameter(OptionParameter::sig),
                                           batch_1.GetParameter(OptionParameter::r), batch_1.GetParameter(OptionParameter::S), batch_1.GetParameter(OptionParameter::b)},
                    {batch_1.T(), batch_1.K(), batch_1.sig(), batch_1.r(), batch_1.S(), batch_1.b()}, 0);
        checkVector("get parameter (perp)", {batch_a.GetParameter(OptionParameter::K), batch_a.GetParameter(OptionParameter::sig),
                                             batch_a.GetParameter(OptionParameter::r), batch_a.GetParameter(OptionParameter::S), batch_a.GetParameter(OptionParameter::b)},
                    {batch_a.K(), batch_a.sig(), batch_a.r(), batch_a.S(), batch_a.b()}, 0);

        // american option: vector constructor, copy, assignment, clone
        AmericanOption am_vector(std::vector<double>{0.25, 65, 0.3, 0.08, 60, 0.08});
        AmericanOption am_copy(batch_1_am);
        AmericanOption am_assigned;
        am_assigned = batch_1_am;
        std::unique_ptr<Option> am_clone = batch_1_am.clone();
        checkValue("vector ctor (am)", am_vector.PutPrice(), batch_1_am.PutPrice(), 0);
        checkValue("copy ctor (am)", am_copy.PutPrice(), batch_1_am.PutPrice(), 0);
        checkValue("assignment (am)", am_assigned.PutPrice(), batch_1_am.PutPrice(), 0);
        checkValue("clone (am)", am_clone->PutPrice(), batch_1_am.PutPrice(), 0);
        checkVector("get parameter (am)", {batch_1_am.GetParameter(OptionParameter::T), batch_1_am.GetParameter(OptionParameter::K), batch_1_am.GetParameter(OptionParameter::sig),
                                           batch_1_am.GetParameter(OptionParameter::r), batch_1_am.GetParameter(OptionParameter::S), batch_1_am.GetParameter(OptionParameter::b)},
                    {batch_1_am.T(), batch_1_am.K(), batch_1_am.sig(), batch_1_am.r(), batch_1_am.S(), batch_1_am.b()}, 0);

        // american option: number of steps is set and kept by copy and assignment
        AmericanOption am_steps(batch_1_am);
        am_steps.Binomial_NSteps(500);
        AmericanOption am_steps_copy(am_steps);
        AmericanOption am_steps_assigned;
        am_steps_assigned = am_steps;
        checkValue("default steps (am)", static_cast<double>(batch_1_am.Binomial_NSteps()), 1000, 0);
        checkValue("steps setter (am)", static_cast<double>(am_steps.Binomial_NSteps()), 500, 0);
        checkValue("copy keeps steps (am)", static_cast<double>(am_steps_copy.Binomial_NSteps()), 500, 0);
        checkValue("assignment keeps steps (am)", static_cast<double>(am_steps_assigned.Binomial_NSteps()), 500, 0);

        // american option: price at an input S equals pricing the option with that S
        AmericanOption am_S70(0.25, 65, 0.3, 0.08, 70, 0.08);
        checkValue("put at input S (am)", batch_1_am.PutPrice(70.0), am_S70.PutPrice(), 0);

        // default matrix has the option's labels and no rows
        MatrixPricer<EuropeanOption> eu_matrix_default;
        MatrixPricer<AmericanPerpetualOption> perp_matrix_default;
        checkValue("matrix default labels", eu_matrix_default.Columns() == EuropeanOption::label_parameters ? 1.0 : 0.0, 1, 0.5);
        checkValue("matrix default rows", static_cast<double>(eu_matrix_default.Data().size()), 0, 0);
        checkValue("perp matrix labels", perp_matrix_default.Columns() == AmericanPerpetualOption::label_parameters ? 1.0 : 0.0, 1, 0.5);

        // batches 1-4 as rows: T, K, sig, r, S, b
        std::vector<std::vector<double>> eu_rows{{0.25, 65, 0.3, 0.08, 60, 0.08},
                                                 {1.0, 100, 0.2, 0.0, 100, 0.0},
                                                 {1.0, 10, 0.5, 0.12, 5, 0.12},
                                                 {30, 100, 0.3, 0.08, 100, 0.08}};
        MatrixPricer<EuropeanOption> eu_matrix(EuropeanOption::label_parameters, eu_rows);
        checkValue("matrix keeps rows", eu_matrix.Data() == eu_rows ? 1.0 : 0.0, 1, 0.5);

        // same batches with shuffled labels: S, K, b, T, r, sig
        std::vector<OptionParameter> shuffled_labels{OptionParameter::S, OptionParameter::K, OptionParameter::b, OptionParameter::T, OptionParameter::r, OptionParameter::sig};
        std::vector<std::vector<double>> shuffled_rows{{60, 65, 0.08, 0.25, 0.08, 0.3},
                                                       {100, 100, 0.0, 1.0, 0.0, 0.2},
                                                       {5, 10, 0.12, 1.0, 0.12, 0.5},
                                                       {100, 100, 0.08, 30, 0.08, 0.3}};
        MatrixPricer<EuropeanOption> eu_matrix_shuffled(shuffled_labels, shuffled_rows);
        checkValue("matrix keeps labels", eu_matrix_shuffled.Columns() == shuffled_labels ? 1.0 : 0.0, 1, 0.5);

        // copy constructor, assignment and setter take both labels and rows
        MatrixPricer<EuropeanOption> eu_matrix_copy(eu_matrix_shuffled);
        MatrixPricer<EuropeanOption> eu_matrix_assigned;
        eu_matrix_assigned = eu_matrix_shuffled;
        MatrixPricer<EuropeanOption> eu_matrix_set;
        eu_matrix_set.Data(shuffled_labels, shuffled_rows);
        checkValue("matrix copy ctor", (eu_matrix_copy.Columns() == shuffled_labels && eu_matrix_copy.Data() == shuffled_rows) ? 1.0 : 0.0, 1, 0.5);
        checkValue("matrix assignment", (eu_matrix_assigned.Columns() == shuffled_labels && eu_matrix_assigned.Data() == shuffled_rows) ? 1.0 : 0.0, 1, 0.5);
        checkValue("matrix setter", (eu_matrix_set.Columns() == shuffled_labels && eu_matrix_set.Data() == shuffled_rows) ? 1.0 : 0.0, 1, 0.5);

        // test display
        std::ostringstream display_stream;
        MatrixPricer<EuropeanOption> eu_matrix_one(EuropeanOption::label_parameters, {{0.25, 65, 0.3, 0.08, 60, 0.08}});
        eu_matrix_one.Display(display_stream);
        checkValue("matrix display", display_stream.str() == "T, K, sig, r, S, b\n0.25, 65, 0.3, 0.08, 60, 0.08\n" ? 1.0 : 0.0, 1, 0.5);

        // every vector function must match its scalar version at each grid point
        std::cout << "--- vector functions ---" << std::endl;

        std::vector<double> grid = Mesher(60, 100, 0.5);

        // batch 1
        checkVector("call price vector", batch_1.CallPrice(grid, OptionParameter::S),
                    applyToEach(grid, [&](double s) { return batch_1.CallPrice(s); }), 0);
        checkVector("put price vector", batch_1.PutPrice(grid, OptionParameter::S),
                    applyToEach(grid, [&](double s) { return batch_1.PutPrice(s); }), 0);

        // batch g
        checkVector("call delta vector", g_test.CallDelta(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0).CallDelta(); }), 0);
        checkVector("put delta vector", g_test.PutDelta(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0).PutDelta(); }), 0);
        checkVector("call gamma vector", g_test.CallGamma(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0).CallGamma(); }), 0);
        checkVector("put gamma vector", g_test.PutGamma(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0).PutGamma(); }), 0);

        // batch with cost of carry
        checkVector("call vega vector", batch_w_carry.CallVega(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0.03).CallVega(); }), 0);
        checkVector("put vega vector", batch_w_carry.PutVega(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0.03).PutVega(); }), 0);
        checkVector("call theta vector", batch_w_carry.CallTheta(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0.03).CallTheta(); }), 0);
        checkVector("put theta vector", batch_w_carry.PutTheta(grid, OptionParameter::S),
                    applyToEach(grid, [](double s) { return EuropeanOption(0.5, 100, 0.36, 0.1, s, 0.03).PutTheta(); }), 0);

        checkVector("DD call delta vector", g_test.DividedDifferencesCallDelta(grid, 1e-4),
                    applyToEach(grid, [&](double s) { return g_test.DividedDifferencesCallDelta(s, 1e-4); }), 0);
        checkVector("DD put delta vector", g_test.DividedDifferencesPutDelta(grid, 1e-4),
                    applyToEach(grid, [&](double s) { return g_test.DividedDifferencesPutDelta(s, 1e-4); }), 0);
        checkVector("DD gamma vector", g_test.DividedDifferencesCallGamma(grid, 1e-2),
                    applyToEach(grid, [&](double s) { return g_test.DividedDifferencesCallGamma(s, 1e-2); }), 0);

        // perpetual batch
        std::vector<double> perp_grid = Mesher(110, 120, 2.5);

        checkVector("perp call delta vector", batch_a.CallDelta(perp_grid, OptionParameter::S),
                    applyToEach(perp_grid, [](double s) { return AmericanPerpetualOption(100, 0.1, 0.1, s, 0.02).CallDelta(); }), 0);
        checkVector("perp call vega vector", batch_a.CallVega(perp_grid, OptionParameter::S),
                    applyToEach(perp_grid, [](double s) { return AmericanPerpetualOption(100, 0.1, 0.1, s, 0.02).CallVega(); }), 0);
        checkVector("perp put vega vector", batch_a.PutVega(perp_grid, OptionParameter::S),
                    applyToEach(perp_grid, [](double s) { return AmericanPerpetualOption(100, 0.1, 0.1, s, 0.02).PutVega(); }), 0);

        std::vector<double> am_grid = Mesher(60, 70, 2.5);
        checkVector("am put vector", batch_1_am.PutPrice(am_grid, OptionParameter::S),
                    applyToEach(am_grid, [](double s) { return AmericanOption(0.25, 65, 0.3, 0.08, s, 0.08).PutPrice(); }), 0);

        // varying parameters other than S
        // expected values from a separate option built with each value
        std::vector<double> grid_K = Mesher(50, 80, 2.5);
        std::vector<double> grid_sig = Mesher(0.1, 0.6, 0.05);
        std::vector<double> grid_T = Mesher(0.1, 2, 0.1);
        std::vector<double> grid_r = Mesher(-0.02, 0.1, 0.02);
        std::vector<double> grid_b = Mesher(-0.04, 0.08, 0.02);

        checkVector("call price vector K", batch_1.CallPrice(grid_K, OptionParameter::K),
                    applyToEach(grid_K, [](double k) { return EuropeanOption(0.25, k, 0.3, 0.08, 60, 0.08).CallPrice(); }), 0);
        checkVector("call price vector r", batch_1.CallPrice(grid_r, OptionParameter::r),
                    applyToEach(grid_r, [](double r) { return EuropeanOption(0.25, 65, 0.3, r, 60, 0.08).CallPrice(); }), 0);
        checkVector("put price vector b", batch_1.PutPrice(grid_b, OptionParameter::b),
                    applyToEach(grid_b, [](double b) { return EuropeanOption(0.25, 65, 0.3, 0.08, 60, b).PutPrice(); }), 0);
        checkVector("call delta vector sig", g_test.CallDelta(grid_sig, OptionParameter::sig),
                    applyToEach(grid_sig, [](double sig) { return EuropeanOption(0.5, 100, sig, 0.1, 105, 0).CallDelta(); }), 0);
        checkVector("put gamma vector T", g_test.PutGamma(grid_T, OptionParameter::T),
                    applyToEach(grid_T, [](double t) { return EuropeanOption(t, 100, 0.36, 0.1, 105, 0).PutGamma(); }), 0);
        checkVector("call vega vector sig", batch_w_carry.CallVega(grid_sig, OptionParameter::sig),
                    applyToEach(grid_sig, [](double sig) { return EuropeanOption(0.5, 100, sig, 0.1, 105, 0.03).CallVega(); }), 0);
        checkVector("put theta vector T", batch_w_carry.PutTheta(grid_T, OptionParameter::T),
                    applyToEach(grid_T, [](double t) { return EuropeanOption(t, 100, 0.36, 0.1, 105, 0.03).PutTheta(); }), 0);
        checkVector("call theta vector r", batch_w_carry.CallTheta(grid_r, OptionParameter::r),
                    applyToEach(grid_r, [](double r) { return EuropeanOption(0.5, 100, 0.36, r, 105, 0.03).CallTheta(); }), 0);
        checkVector("perp put vector sig", batch_a.PutPrice(grid_sig, OptionParameter::sig),
                    applyToEach(grid_sig, [](double sig) { return AmericanPerpetualOption(100, sig, 0.1, 110, 0.02).PutPrice(); }), 0);
        checkVector("perp call vega vector sig", batch_a.CallVega(grid_sig, OptionParameter::sig),
                    applyToEach(grid_sig, [](double sig) { return AmericanPerpetualOption(100, sig, 0.1, 110, 0.02).CallVega(); }), 0);

        // every row of eu_matrix (batches 1-4) must match pricing that batch directly
        checkVector("matrix call price", MatrixPrice(eu_matrix, OptionTypeParameter::Call), {batch_1.CallPrice(), batch_2.CallPrice(), batch_3.CallPrice(), batch_4.CallPrice()}, 0);
        checkVector("matrix put price", MatrixPrice(eu_matrix, OptionTypeParameter::Put), {batch_1.PutPrice(), batch_2.PutPrice(), batch_3.PutPrice(), batch_4.PutPrice()}, 0);
        checkVector("matrix call delta", MatrixDelta(eu_matrix, OptionTypeParameter::Call), {batch_1.CallDelta(), batch_2.CallDelta(), batch_3.CallDelta(), batch_4.CallDelta()}, 0);
        checkVector("matrix put delta", MatrixDelta(eu_matrix, OptionTypeParameter::Put), {batch_1.PutDelta(), batch_2.PutDelta(), batch_3.PutDelta(), batch_4.PutDelta()}, 0);
        checkVector("matrix call gamma", MatrixGamma(eu_matrix, OptionTypeParameter::Call), {batch_1.CallGamma(), batch_2.CallGamma(), batch_3.CallGamma(), batch_4.CallGamma()}, 0);
        checkVector("matrix put gamma", MatrixGamma(eu_matrix, OptionTypeParameter::Put), {batch_1.PutGamma(), batch_2.PutGamma(), batch_3.PutGamma(), batch_4.PutGamma()}, 0);
        checkVector("matrix call vega", MatrixVega(eu_matrix, OptionTypeParameter::Call), {batch_1.CallVega(), batch_2.CallVega(), batch_3.CallVega(), batch_4.CallVega()}, 0);
        checkVector("matrix put vega", MatrixVega(eu_matrix, OptionTypeParameter::Put), {batch_1.PutVega(), batch_2.PutVega(), batch_3.PutVega(), batch_4.PutVega()}, 0);
        checkVector("matrix call theta", MatrixTheta(eu_matrix, OptionTypeParameter::Call), {batch_1.CallTheta(), batch_2.CallTheta(), batch_3.CallTheta(), batch_4.CallTheta()}, 0);
        checkVector("matrix put theta", MatrixTheta(eu_matrix, OptionTypeParameter::Put), {batch_1.PutTheta(), batch_2.PutTheta(), batch_3.PutTheta(), batch_4.PutTheta()}, 0);

        // shuffled labels must give the same options
        checkVector("matrix shuffled labels", MatrixPrice(eu_matrix_shuffled, OptionTypeParameter::Put), MatrixPrice(eu_matrix, OptionTypeParameter::Put), 0);

        // perpetual rows: K, sig, r, S, b
        MatrixPricer<AmericanPerpetualOption> perp_matrix(AmericanPerpetualOption::label_parameters, {{100, 0.1, 0.1, 110, 0.02}, {100, 0.1, 0.1, 120, 0.02}, {90, 0.2, 0.08, 100, 0.03}});
        AmericanPerpetualOption perp_row_1(100, 0.1, 0.1, 110, 0.02);
        AmericanPerpetualOption perp_row_2(100, 0.1, 0.1, 120, 0.02);
        AmericanPerpetualOption perp_row_3(90, 0.2, 0.08, 100, 0.03);

        checkVector("perp matrix call", MatrixPrice(perp_matrix, OptionTypeParameter::Call), {perp_row_1.CallPrice(), perp_row_2.CallPrice(), perp_row_3.CallPrice()}, 0);
        checkVector("perp matrix put", MatrixPrice(perp_matrix, OptionTypeParameter::Put), {perp_row_1.PutPrice(), perp_row_2.PutPrice(), perp_row_3.PutPrice()}, 0);
        checkVector("perp matrix delta", MatrixDelta(perp_matrix, OptionTypeParameter::Call), {perp_row_1.CallDelta(), perp_row_2.CallDelta(), perp_row_3.CallDelta()}, 0);
        checkVector("perp matrix vega", MatrixVega(perp_matrix, OptionTypeParameter::Put), {perp_row_1.PutVega(), perp_row_2.PutVega(), perp_row_3.PutVega()}, 0);

        // test empty matrix gives an empty result
        // pricing does not change the matrix
        checkValue("matrix empty", static_cast<double>(MatrixPrice(eu_matrix_default, OptionTypeParameter::Call).size()), 0, 0);
        checkValue("matrix keeps data", eu_matrix.Data() == eu_rows ? 1.0 : 0.0, 1, 0.5);

        // grid over K = {60, 65, 70} and sig = {0.2, 0.3}
        // outer loop over K, other parameters from batch 1
        MatrixPricer<EuropeanOption> eu_grid = MatrixPricer<EuropeanOption>::Grid(batch_1, OptionParameter::K, {60, 65, 70}, OptionParameter::sig, {0.2, 0.3});
        std::vector<std::vector<double>> grid_rows{{0.25, 60, 0.2, 0.08, 60, 0.08},
                                                   {0.25, 60, 0.3, 0.08, 60, 0.08},
                                                   {0.25, 65, 0.2, 0.08, 60, 0.08},
                                                   {0.25, 65, 0.3, 0.08, 60, 0.08},
                                                   {0.25, 70, 0.2, 0.08, 60, 0.08},
                                                   {0.25, 70, 0.3, 0.08, 60, 0.08}};

        checkValue("grid size", static_cast<double>(eu_grid.Data().size()), 6, 0);
        checkValue("grid labels", eu_grid.Columns() == EuropeanOption::label_parameters ? 1.0 : 0.0, 1, 0.5);
        checkValue("grid rows", eu_grid.Data() == grid_rows ? 1.0 : 0.0, 1, 0.5);
        checkVector("grid call price", MatrixPrice(eu_grid, OptionTypeParameter::Call),
                    {EuropeanOption(0.25, 60, 0.2, 0.08, 60, 0.08).CallPrice(), EuropeanOption(0.25, 60, 0.3, 0.08, 60, 0.08).CallPrice(),
                     EuropeanOption(0.25, 65, 0.2, 0.08, 60, 0.08).CallPrice(), EuropeanOption(0.25, 65, 0.3, 0.08, 60, 0.08).CallPrice(),
                     EuropeanOption(0.25, 70, 0.2, 0.08, 60, 0.08).CallPrice(), EuropeanOption(0.25, 70, 0.3, 0.08, 60, 0.08).CallPrice()}, 0);

        // perpetual grid over S = {100, 110, 120} and sig = {0.1, 0.15}
        // inside critical price boundaries
        MatrixPricer<AmericanPerpetualOption> perp_grid_matrix = MatrixPricer<AmericanPerpetualOption>::Grid(batch_a, OptionParameter::S, {100, 110, 120}, OptionParameter::sig, {0.1, 0.15});
        checkValue("perp grid labels", perp_grid_matrix.Columns() == AmericanPerpetualOption::label_parameters ? 1.0 : 0.0, 1, 0.5);
        checkVector("perp grid put", MatrixPrice(perp_grid_matrix, OptionTypeParameter::Put),
                    {AmericanPerpetualOption(100, 0.1, 0.1, 100, 0.02).PutPrice(), AmericanPerpetualOption(100, 0.15, 0.1, 100, 0.02).PutPrice(),
                     AmericanPerpetualOption(100, 0.1, 0.1, 110, 0.02).PutPrice(), AmericanPerpetualOption(100, 0.15, 0.1, 110, 0.02).PutPrice(),
                     AmericanPerpetualOption(100, 0.1, 0.1, 120, 0.02).PutPrice(), AmericanPerpetualOption(100, 0.15, 0.1, 120, 0.02).PutPrice()}, 0);

        // test reshaper
        // flat values cut into rows of values 1, columns of values 2
        checkValue("reshape 3x2", Reshaper({1, 2, 3, 4, 5, 6}, 3, 2) == std::vector<std::vector<double>>{{1, 2}, {3, 4}, {5, 6}} ? 1.0 : 0.0, 1, 0.5);
        checkValue("reshape 1x6", Reshaper({1, 2, 3, 4, 5, 6}, 1, 6) == std::vector<std::vector<double>>{{1, 2, 3, 4, 5, 6}} ? 1.0 : 0.0, 1, 0.5);

        // test table
        // grid, matrix function and reshape in one call
        std::vector<std::vector<double>> price_table = MatrixTable<EuropeanOption>(batch_1, OptionParameter::K, {60, 65, 70}, OptionParameter::sig, {0.2, 0.3}, MatrixPrice<EuropeanOption>, OptionTypeParameter::Call);
        std::vector<std::vector<double>> delta_table = MatrixTable<EuropeanOption>(batch_1, OptionParameter::K, {60, 65, 70}, OptionParameter::sig, {0.2, 0.3}, MatrixDelta<EuropeanOption>, OptionTypeParameter::Put);
        std::vector<std::vector<double>> perp_vega_table = MatrixTable<AmericanPerpetualOption>(batch_a, OptionParameter::S, {100, 110, 120}, OptionParameter::sig, {0.1, 0.15}, MatrixVega<AmericanPerpetualOption>, OptionTypeParameter::Call);

        checkValue("table call price", price_table == Reshaper(MatrixPrice(eu_grid, OptionTypeParameter::Call), 3, 2) ? 1.0 : 0.0, 1, 0.5);
        checkValue("table put delta [1][0]", delta_table[1][0], EuropeanOption(0.25, 65, 0.2, 0.08, 60, 0.08).PutDelta(), 0);
        checkValue("perp table vega [2][1]", perp_vega_table[2][1], AmericanPerpetualOption(100, 0.15, 0.1, 120, 0.02).CallVega(), 0);

        // test csv
        // write a matrix, read it back, get exactly the same labels and rows
        // (1/3 needs double precision)
        MatrixPricer<EuropeanOption> csv_matrix(EuropeanOption::label_parameters, {{0.25, 65, 0.3, 0.08, 60, 0.08}, {1.0 / 3.0, 100, 0.2, 0.05, 100, 0.05}});
        csv_matrix.WriteCSV("test_matrix.csv");
        MatrixPricer<EuropeanOption> csv_read = MatrixPricer<EuropeanOption>::ReadCSV("test_matrix.csv");
        checkValue("csv labels", csv_read.Columns() == EuropeanOption::label_parameters ? 1.0 : 0.0, 1, 0.5);
        checkValue("csv rows", csv_read.Data() == csv_matrix.Data() ? 1.0 : 0.0, 1, 0.5);

        // shuffled labels come back in the same order and give the same prices
        eu_matrix_shuffled.WriteCSV("test_shuffled.csv");
        MatrixPricer<EuropeanOption> csv_shuffled = MatrixPricer<EuropeanOption>::ReadCSV("test_shuffled.csv");
        checkValue("csv shuffled labels", csv_shuffled.Columns() == shuffled_labels ? 1.0 : 0.0, 1, 0.5);
        checkVector("csv shuffled prices", MatrixPrice(csv_shuffled, OptionTypeParameter::Put), MatrixPrice(eu_matrix, OptionTypeParameter::Put), 0);

        // perpetual matrix
        perp_matrix.WriteCSV("test_perp.csv");
        checkValue("csv perp rows", MatrixPricer<AmericanPerpetualOption>::ReadCSV("test_perp.csv").Data() == perp_matrix.Data() ? 1.0 : 0.0, 1, 0.5);

        // windows line endings, spaces and blank lines are read the same as a clean file
        writeFile("test_windows.csv", "T,K,sig,r,S,b\r\n0.25,65,0.3,0.08,60,0.08\r\n");
        writeFile("test_messy.csv", "T, K, sig, r, S, b\n\n 0.25, 65, 0.3, 0.08, 60, 0.08 \n\n");
        checkValue("csv windows endings", MatrixPricer<EuropeanOption>::ReadCSV("test_windows.csv").Data() == std::vector<std::vector<double>>{{0.25, 65, 0.3, 0.08, 60, 0.08}} ? 1.0 : 0.0, 1, 0.5);
        checkValue("csv spaces, blank lines", MatrixPricer<EuropeanOption>::ReadCSV("test_messy.csv").Data() == std::vector<std::vector<double>>{{0.25, 65, 0.3, 0.08, 60, 0.08}} ? 1.0 : 0.0, 1, 0.5);

        // the option itself is not changed by the vector functions
        checkValue("vector keeps option", batch_1.CallPrice(), eu_copy.CallPrice(), 0);

        // test exception handling
        std::cout << "--- exceptions ---" << std::endl;

        checkThrows("EU ctor T = 0", [] { EuropeanOption bad(0, 100, 0.2, 0.05, 100, 0.05); });
        checkThrows("EU ctor K = 0", [] { EuropeanOption bad(1, 0, 0.2, 0.05, 100, 0.05); });
        checkThrows("EU ctor sig = 0", [] { EuropeanOption bad(1, 100, 0, 0.05, 100, 0.05); });
        checkThrows("EU ctor S = 0", [] { EuropeanOption bad(1, 100, 0.2, 0.05, 0, 0.05); });
        checkThrows("perp ctor K = 0", [] { AmericanPerpetualOption bad(0, 0.1, 0.1, 110, 0.02); });

        checkThrows("EU vector ctor size 4", [] { EuropeanOption bad(std::vector<double>{1, 100, 0.2, 0.05}); });
        checkThrows("EU vector ctor size 5", [] { EuropeanOption bad(std::vector<double>{100, 0.2, 0.05, 100, 0.05}); });
        checkThrows("EU vector ctor K = 0", [] { EuropeanOption bad(std::vector<double>{1, 0, 0.2, 0.05, 100, 0.05}); });
        checkThrows("perp vector ctor size 6", [] { AmericanPerpetualOption bad(std::vector<double>{999, 100, 0.1, 0.1, 110, 0.02}); });

        checkThrows("setter T = 0", [&] { EuropeanOption option(batch_1); option.T(0); });
        checkThrows("setter K = 0", [&] { EuropeanOption option(batch_1); option.K(0); });
        checkThrows("setter sig = 0", [&] { EuropeanOption option(batch_1); option.sig(0); });
        checkThrows("setter S = 0", [&] { EuropeanOption option(batch_1); option.S(0); });

        checkThrows("DD h = 0", [&] { g_test.DividedDifferencesCallDelta(0.0); });
        checkThrows("DD h >= S", [&] { g_test.DividedDifferencesCallDelta(200.0); });
        checkThrows("DD vector S = 0", [&] { g_test.DividedDifferencesCallDelta(std::vector<double>{100, 0}, 1e-4); });
        checkThrows("price vector S = 0", [&] { batch_1.CallPrice(std::vector<double>{60, 0, 70}, OptionParameter::S); });
        checkThrows("delta vector S = 0", [&] { g_test.CallDelta(std::vector<double>{100, 0}, OptionParameter::S); });
        checkThrows("price vector sig = 0", [&] { batch_1.CallPrice(std::vector<double>{0.2, 0}, OptionParameter::sig); });
        checkThrows("vega vector sig = 0", [&] { batch_w_carry.CallVega(std::vector<double>{0.2, 0}, OptionParameter::sig); });
        checkThrows("perp vector T", [&] { batch_a.CallPrice(std::vector<double>{1, 2}, OptionParameter::T); });
        checkThrows("perp vega vector T", [&] { batch_a.CallVega(std::vector<double>{1, 2}, OptionParameter::T); });
        checkThrows("perp get parameter T", [&] { batch_a.GetParameter(OptionParameter::T); });

        checkThrows("am ctor T = 0", [] { AmericanOption bad(0, 65, 0.3, 0.08, 60, 0.08); });
        checkThrows("am vector ctor size 5", [] { AmericanOption bad(std::vector<double>{65, 0.3, 0.08, 60, 0.08}); });
        checkThrows("am setter T = 0", [&] { AmericanOption option(batch_1_am); option.T(0); });
        checkThrows("am steps = 2", [&] { AmericanOption option(batch_1_am); option.Binomial_NSteps(2); option.PutPrice(); });

        checkThrows("engine N = 2", [&] { BinomialEngine<EuropeanOption> engine(batch_1, CRR(), 2); });
        checkThrows("engine setter N = 2", [&] { BinomialEngine<EuropeanOption> engine(batch_1, CRR(), 1000); engine.N(2); });

        checkThrows("matrix label missing", [] { MatrixPricer<EuropeanOption> bad({OptionParameter::T, OptionParameter::K, OptionParameter::sig, OptionParameter::r, OptionParameter::S}, {}); });
        checkThrows("matrix label twice", [] { MatrixPricer<EuropeanOption> bad({OptionParameter::T, OptionParameter::K, OptionParameter::sig, OptionParameter::r, OptionParameter::S, OptionParameter::b, OptionParameter::b}, {}); });
        checkThrows("matrix label repeated", [] { MatrixPricer<EuropeanOption> bad({OptionParameter::T, OptionParameter::K, OptionParameter::sig, OptionParameter::r, OptionParameter::S, OptionParameter::S}, {}); });
        checkThrows("perp matrix label T", [] { MatrixPricer<AmericanPerpetualOption> bad(EuropeanOption::label_parameters, {}); });
        checkThrows("matrix row size", [] { MatrixPricer<EuropeanOption> bad(EuropeanOption::label_parameters, {{0.25, 65, 0.3, 0.08, 60}}); });
        checkThrows("matrix setter labels", [] { MatrixPricer<EuropeanOption> matrix; matrix.Data({OptionParameter::T, OptionParameter::K}, {}); });
        checkThrows("matrix row sig = 0", [] { MatrixPricer<EuropeanOption> matrix(EuropeanOption::label_parameters, {{0.25, 65, 0, 0.08, 60, 0.08}}); MatrixPrice(matrix, OptionTypeParameter::Call); });
        checkThrows("perp matrix b > r", [] { MatrixPricer<AmericanPerpetualOption> matrix(AmericanPerpetualOption::label_parameters, {{100, 0.1, 0.1, 110, 0.15}}); MatrixPrice(matrix, OptionTypeParameter::Call); });

        checkThrows("grid same parameter", [&] { MatrixPricer<EuropeanOption>::Grid(batch_1, OptionParameter::K, {60, 65}, OptionParameter::K, {70}); });
        checkThrows("perp grid T", [&] { MatrixPricer<AmericanPerpetualOption>::Grid(batch_a, OptionParameter::T, {1, 2}, OptionParameter::S, {100}); });
        checkThrows("grid empty values", [&] { MatrixPricer<EuropeanOption>::Grid(batch_1, OptionParameter::K, {}, OptionParameter::sig, {0.2}); });
        checkThrows("reshape wrong size", [] { Reshaper({1, 2, 3, 4, 5}, 3, 2); });
        checkThrows("table same parameter", [&] { MatrixTable<EuropeanOption>(batch_1, OptionParameter::sig, {0.2}, OptionParameter::sig, {0.3}, MatrixPrice<EuropeanOption>, OptionTypeParameter::Call); });
        checkThrows("table sig = 0", [&] { MatrixTable<EuropeanOption>(batch_1, OptionParameter::K, {60}, OptionParameter::sig, {0.2, 0}, MatrixGamma<EuropeanOption>, OptionTypeParameter::Call); });

        writeFile("test_empty.csv", "");
        writeFile("test_label.csv", "T,K,sigma,r,S,b\n0.25,65,0.3,0.08,60,0.08\n");
        writeFile("test_junk.csv", "T,K,sig,r,S,b\n0.25,65,0.3abc,0.08,60,0.08\n");
        writeFile("test_gap.csv", "T,K,sig,r,S,b\n0.25,65,,0.08,60,0.08\n");
        writeFile("test_short.csv", "T,K,sig,r,S,b\n0.25,65,0.3,0.08,60\n");
        checkThrows("csv missing file", [] { MatrixPricer<EuropeanOption>::ReadCSV("no_such_file.csv"); });
        checkThrows("csv empty file", [] { MatrixPricer<EuropeanOption>::ReadCSV("test_empty.csv"); });
        checkThrows("csv unknown label", [] { MatrixPricer<EuropeanOption>::ReadCSV("test_label.csv"); });
        checkThrows("csv junk value", [] { MatrixPricer<EuropeanOption>::ReadCSV("test_junk.csv"); });
        checkThrows("csv empty value", [] { MatrixPricer<EuropeanOption>::ReadCSV("test_gap.csv"); });
        checkThrows("csv short row", [] { MatrixPricer<EuropeanOption>::ReadCSV("test_short.csv"); });
        checkThrows("perp csv label T", [] { MatrixPricer<AmericanPerpetualOption>::ReadCSV("test_junk.csv"); });
        checkThrows("csv write bad path", [] { MatrixPricer<EuropeanOption>().WriteCSV("no_such_folder/test.csv"); });
        std::remove("test_empty.csv");
        std::remove("test_label.csv");
        std::remove("test_junk.csv");
        std::remove("test_gap.csv");
        std::remove("test_short.csv");

        checkThrows("mesher step 0", [] { Mesher(60, 100, 0); });
        checkThrows("mesher wrong sign", [] { Mesher(60, 100, -0.5); });

        // test exercise boundary
        std::cout << "--- perpetual boundary ---" << std::endl;

        checkValue("perp C(200)", batch_a.CallPrice(200.0), 100, 1e-12);
        checkValue("perp P(50)", batch_a.PutPrice(50.0), 50, 1e-12);

        // the greeks past the boundary
        AmericanPerpetualOption perp_deep_call(100, 0.1, 0.1, 200, 0.02);
        AmericanPerpetualOption perp_deep_put(100, 0.1, 0.1, 50, 0.02);
        checkValue("perp call delta S=200", perp_deep_call.CallDelta(), 1, 1e-8);
        checkValue("perp call gamma S=200", perp_deep_call.CallGamma(), 0, 1e-8);
        checkValue("perp put delta S=50", perp_deep_put.PutDelta(), -1, 1e-8);
        checkValue("perp put gamma S=50", perp_deep_put.PutGamma(), 0, 1e-8);
        checkValue("perp call vega S=200", perp_deep_call.CallVega(), 0, 0);
        checkValue("perp put vega S=50", perp_deep_put.PutVega(), 0, 0);

        // test critical prices
        checkValue("perp S*", batch_a.CriticalPriceCall(), 145.10619103767687, 1e-8);
        checkValue("perp S**", batch_a.CriticalPricePut(), 86.1438089623231, 1e-8);

        // test value matching at the boundary
        checkValue("perp C(S*) = S* - K", batch_a.CallPrice(batch_a.CriticalPriceCall()), batch_a.CriticalPriceCall() - batch_a.K(), 1e-8);
        checkValue("perp P(S**) = K - S**", batch_a.PutPrice(batch_a.CriticalPricePut()), batch_a.K() - batch_a.CriticalPricePut(), 1e-8);

        // test delta just inside the boundary is +-1
        AmericanPerpetualOption perp_below_call(100, 0.1, 0.1, batch_a.CriticalPriceCall() - 1e-6, 0.02);
        AmericanPerpetualOption perp_above_put(100, 0.1, 0.1, batch_a.CriticalPricePut() + 1e-6, 0.02);
        checkValue("perp call delta below S*", perp_below_call.CallDelta(), 1, 1e-6);
        checkValue("perp put delta above S**", perp_above_put.PutDelta(), -1, 1e-6);

        // gamma jumps at the boundary
        checkValue("perp call gamma below S*", perp_below_call.CallGamma(), 0.0152784, 1e-6);

        // test a grid crossing both boundaries
        std::vector<double> perp_wide_grid = Mesher(50, 200, 2.5);
        checkVector("perp call across S*", batch_a.CallPrice(perp_wide_grid, OptionParameter::S),
                    applyToEach(perp_wide_grid, [&](double s) { return batch_a.CallPrice(s); }), 0);
        checkVector("perp put across S**", batch_a.PutPrice(perp_wide_grid, OptionParameter::S),
                    applyToEach(perp_wide_grid, [&](double s) { return batch_a.PutPrice(s); }), 0);

        // test parameters for which the perpetual formula is valid
        std::cout << "--- perpetual validity ---" << std::endl;

        // call needs b < r
        // put needs r > 0
        checkThrows("perp call b = r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.1); option.CallPrice(); });
        checkThrows("perp call b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CallPrice(); });
        checkThrows("perp put r = 0", [] { AmericanPerpetualOption option(100, 0.1, 0.0, 110, 0.02); option.PutPrice(); });
        checkThrows("perp put r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 110, 0.02); option.PutPrice(); });

        // greeks and critical prices follow the same rule
        checkThrows("perp call delta b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CallDelta(); });
        checkThrows("perp call gamma b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CallGamma(); });
        checkThrows("perp put delta r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 50, 0.02); option.PutDelta(); });
        checkThrows("perp put gamma r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 50, 0.02); option.PutGamma(); });
        checkThrows("perp call vega b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CallVega(); });
        checkThrows("perp put vega r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 110, 0.02); option.PutVega(); });
        checkThrows("perp call theta b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CallTheta(); });
        checkThrows("perp put theta r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 110, 0.02); option.PutTheta(); });
        checkThrows("perp S* b > r", [] { AmericanPerpetualOption option(100, 0.1, 0.1, 110, 0.15); option.CriticalPriceCall(); });
        checkThrows("perp S** r < 0", [] { AmericanPerpetualOption option(100, 0.1, -0.01, 110, 0.02); option.CriticalPricePut(); });

        // test invalid setter
        checkThrows("perp setter results b > r", [&] { AmericanPerpetualOption option(batch_a); option.b(0.2); option.CallPrice(); });
        checkThrows("perp setter results r < 0", [&] { AmericanPerpetualOption option(batch_a); option.r(-0.01); option.PutPrice(); });

        std::cout << "--- mesher ---" << std::endl;

        // test meshers
        checkValue("Mesher(1,1.3,0.1) size", static_cast<double>(Mesher(1, 1.3, 0.1).size()), 4, 0);
        checkValue("Mesher(.2,.21,.01) size", static_cast<double>(Mesher(0.2, 0.21, 0.01).size()), 2, 0);
        checkValue("Mesher(5,5,1) size", static_cast<double>(Mesher(5, 5, 1).size()), 1, 0);

        // decreasing grid
        std::vector<double> grid_decreasing = Mesher(100, 60, -0.5);
        checkVector("decreasing grid", grid_decreasing, std::vector<double>(grid.rbegin(), grid.rend()), 0);
    }
    catch (const std::exception& error)
    {
        std::cout << error.what() << std::endl;
        fail_counter++;
    }
    catch (...)
    {
        std::cout << "unhandled exception" << std::endl;
        fail_counter++;
    }

    std::cout << checks_counter << " number of checks, " << fail_counter << " failed" << std::endl;
    return fail_counter > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
