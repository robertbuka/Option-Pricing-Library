//
//  Option.cpp
//  option_pricing_lib
//
//  implementation of Option abstract base class functionality

#include "Option.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <sstream>

namespace option_pricing_lib
{
Option::Option(): m_K(1), m_sig(1), m_r(0), m_S(1), m_b(0) {} // default constructor, positive parameters are set to 1

Option::Option(const Option& reference): m_K(reference.m_K), m_sig(reference.m_sig), m_r(reference.m_r), m_S(reference.m_S), m_b(reference.m_b) {} // copy constructor

Option::Option(double K, double sig, double r, double S, double b): m_K(K), m_sig(sig), m_r(r), m_S(S), m_b(b) // custom constructor
{
    // check for invalid arguments
    if (K <= 0 || sig <= 0 || S <= 0) throw std::invalid_argument("Option (base class)| Parametric constructor: K, sig, and S must be > 0");
}

Option::Option(const std::vector<double>& properties_array)
{// custom constructor with vector as input
    // note that values in properties_array must be in the following order:
    // K, sig, r, S, b
    
    // check if input array has appropriate size
    if (properties_array.size() != 5)
    {
        throw std::invalid_argument("Option (base class)| Array constructor: input array size must be equal to 5");
    }
    
    // check for invalid arguments
    if (properties_array[0] <= 0 || properties_array[1] <= 0 || properties_array[3] <= 0) throw std::invalid_argument("Option (base class)| Array constructor: K, sig, and S must be > 0");
    
    m_K = properties_array[0];
    m_sig = properties_array[1];
    m_r = properties_array[2];
    m_S = properties_array[3];
    m_b = properties_array[4];
}

Option::~Option() {} // destructor

double Option::K() const
{// return strike price
    return m_K;
}

double Option::sig() const
{// return volatility
    return m_sig;
}

double Option::r() const
{// return risk-free interest rate
    return m_r;
}

double Option::S() const
{// return current stock price
    return m_S;
}

double Option::b() const
{// return cost of carry
    return m_b;
}

void Option::K(double value)
{// set strike price to value
    
    // check for invalid argument
    if (value <= 0) throw std::invalid_argument("Option (base class)| Setter: K must be > 0");
    
    m_K = value;
}

void Option::sig(double value)
{// set volatility to value
    
    // check for invalid argument
    if (value <= 0) throw std::invalid_argument("Option (base class)| Setter: sig must be > 0");
    
    m_sig = value;
}

void Option::r(double value)
{// set risk-free interest rate to value
    m_r = value;
}

void Option::S(double value)
{// set current stock price to value
    
    // check for invalid argument
    if (value <= 0) throw std::invalid_argument("Option (base class)| Setter: S must be > 0");
    
    m_S = value;
}

void Option::b(double value)
{// set cost of carry to value
    m_b = value;
}

Option& Option::operator = (const Option& reference)
{// assignment operator
    
    if (&reference == this) return *this; // check for self assignment
    
    m_K = reference.m_K;
    m_sig = reference.m_sig;
    m_r = reference.m_r;
    m_S = reference.m_S;
    m_b = reference.m_b;
    
    return *this;
}

double Option::DividedDifferencesCallDelta(double h) const
{// return call delta using divided differences
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= m_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    // calculate V(S + h)
    double value_s_plus_h = CallPrice(m_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = CallPrice(m_S - h);
    
    // return (V(S + h) - V(S - h)) / 2h
    
    return (value_s_plus_h - value_s_minus_h) / (2 * h);
}

double Option::DividedDifferencesPutDelta(double h) const
{// return put delta using divided differences
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= m_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    // calculate V(S + h)
    double value_s_plus_h = PutPrice(m_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = PutPrice(m_S - h);
    
    // return (V(S + h) - V(S - h)) / 2h
    
    return (value_s_plus_h - value_s_minus_h) / (2 * h);
}

double Option::DividedDifferencesCallGamma(double h) const
{// return call gamma using divided differences
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= m_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    
    // calculate V(S + h)
    double value_s_plus_h = CallPrice(m_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = CallPrice(m_S - h);
    
    // return (V(S + h) - 2 * V(S) + V(S - h)) / h^2
    
    return (value_s_plus_h + value_s_minus_h - 2 * CallPrice()) / (h * h);
}

double Option::DividedDifferencesPutGamma(double h) const
{// return put gamma using divided differences
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= m_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    
    // calculate V(S + h)
    double value_s_plus_h = PutPrice(m_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = PutPrice(m_S - h);
    
    // return (V(S + h) - 2 * V(S) + V(S - h)) / h^2
    
    return (value_s_plus_h + value_s_minus_h - 2 * PutPrice()) / (h * h);
}

double Option::DividedDifferencesCallDelta(double input_S, double h) const
{// return call delta using input underlying value
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= input_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    // calculate V(S + h)
    double value_s_plus_h = CallPrice(input_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = CallPrice(input_S - h);
    
    // return (V(S + h) - V(S - h)) / 2h
    
    return (value_s_plus_h - value_s_minus_h) / (2 * h);
}

double Option::DividedDifferencesPutDelta(double input_S, double h) const
{// return put delta using input underlying value
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= input_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    // calculate V(S + h)
    double value_s_plus_h = PutPrice(input_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = PutPrice(input_S - h);
    
    // return (V(S + h) - V(S - h)) / 2h
    
    return (value_s_plus_h - value_s_minus_h) / (2 * h);
}

double Option::DividedDifferencesCallGamma(double input_S, double h) const
{// return call gamma using input underlying value
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= input_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    
    // calculate V(S + h)
    double value_s_plus_h = CallPrice(input_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = CallPrice(input_S - h);
    
    // return (V(S + h) - 2 * V(S) + V(S - h)) / h^2
    
    return (value_s_plus_h + value_s_minus_h - 2 * CallPrice(input_S)) / (h * h);
}

double Option::DividedDifferencesPutGamma(double input_S, double h) const
{// return put gamma using input underlying value
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    if (h >= input_S) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be < S");
    
    
    // calculate V(S + h)
    double value_s_plus_h = PutPrice(input_S + h);
    
    // calculate V(S - h)
    double value_s_minus_h = PutPrice(input_S - h);
    
    // return (V(S + h) - 2 * V(S) + V(S - h)) / h^2
    
    return (value_s_plus_h + value_s_minus_h - 2 * PutPrice(input_S)) / (h * h);
}

std::vector<double> Option::DividedDifferencesCallDelta(const std::vector<double>& input_S, double h) const
{// return a vector of call delta based on input_S (vector of underlying values)
    return DDHelper(input_S, h, [this](double s, double step){ return DividedDifferencesCallDelta(s,step); });
}

std::vector<double> Option::DividedDifferencesPutDelta(const std::vector<double>& input_S, double h) const
{// return a vector of put delta based on input_S (vector of underlying values)
    return DDHelper(input_S, h, [this](double s, double step){ return DividedDifferencesPutDelta(s,step); });
}

std::vector<double> Option::DividedDifferencesCallGamma(const std::vector<double>& input_S, double h) const
{// return a vector of call gamma based on input_S (vector of underlying values)
    return DDHelper(input_S, h, [this](double s, double step){ return DividedDifferencesCallGamma(s,step); });
}

std::vector<double> Option::DividedDifferencesPutGamma(const std::vector<double>& input_S, double h) const
{// return a vector of put gamma based on input_S (vector of underlying values)
    return DDHelper(input_S, h, [this](double s, double step){ return DividedDifferencesPutGamma(s,step); });
}

std::vector<double> Option::CallPrice(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of call option prices based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.CallPrice(); });
}

std::vector<double> Option::PutPrice(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of put option prices based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.PutPrice(); });
}

std::vector<double> Option::DDHelper(const std::vector<double>& input_S, double h, const std::function<double(double,double)>& dd_func) const
{// helper function for divided differences method
    // check for invalid input_S
    for (std::vector<double>::const_iterator input_S_it = input_S.begin(); input_S_it != input_S.end(); ++input_S_it)
    {
        if (*input_S_it <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: S must always be > 0");
    }
    
    // check for invalid argument
    if (h <= 0) throw std::invalid_argument("Option (base class)| Divided Differences Method: h must be > 0");
    // h against every input_S value is checked inside helper function
    
    // create output vector
    std::vector<double> output_vector;
    
    // fill output vector
    for (std::vector<double>::const_iterator input_S_it = input_S.begin(); input_S_it != input_S.end(); ++input_S_it)
    {
        output_vector.push_back(dd_func(*input_S_it,h));
    }
    
    return output_vector;
}

std::vector<double> Option::CallDelta(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of call delta based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.CallDelta(); });
}

std::vector<double> Option::PutDelta(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of put delta based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.PutDelta(); });
}

std::vector<double> Option::CallGamma(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of call gamma based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.CallGamma(); });
}

std::vector<double> Option::PutGamma(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of put gamma based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.PutGamma(); });
}

std::vector<double> Option::CallVega(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of call vega based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.CallVega(); });
}

std::vector<double> Option::PutVega(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of put vega based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.PutVega(); });
}

std::vector<double> Option::CallTheta(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of call theta based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.CallTheta(); });
}

std::vector<double> Option::PutTheta(const std::vector<double>& input, OptionParameter parameter) const
{// return a vector of put vega based on input (vector of option parameter values)
    return VPHelper(input, parameter, [](const Option& internal_option){return internal_option.PutTheta(); });
}

std::vector<double> Option::VPHelper(const std::vector<double>& input, OptionParameter parameter, const std::function<double(const Option&)>& vp_func) const
{
    // create output vector and internal option instance
    std::vector<double> option_price_vector;
    std::unique_ptr<Option> internal_option_ptr = clone();
    
    // reserve capacity for output vector
    option_price_vector.reserve(input.size());
    
    // fill output vector
    for (std::vector<double>::const_iterator input_it = input.begin(); input_it != input.end(); ++input_it)
    {
        internal_option_ptr->SetParameter(parameter, *input_it);
        option_price_vector.push_back(vp_func(*internal_option_ptr));
    }
    
    return option_price_vector;
}

std::ostream& operator << (std::ostream& os, OptionParameter parameter)
{
    switch (parameter)
    {
        case OptionParameter::T:
            return os << "T";
            break;
        case OptionParameter::K:
            return os << "K";
            break;
        case OptionParameter::sig:
            return os << "sig";
            break;
        case OptionParameter::r:
            return os << "r";
            break;
        case OptionParameter::S:
            return os << "S";
            break;
        case OptionParameter::b:
            return os << "b";
            break;
    }
    throw std::invalid_argument("OptionParameter| invalid option parameter");
}

std::istream& operator >> (std::istream& is, OptionParameter& parameter)
{
    std::string s;
    is >> s;
    if (s == "T") {
        parameter = OptionParameter::T;
        return is;
    } else if (s == "K"){
        parameter = OptionParameter::K;
        return is;
    } else if (s == "sig"){
        parameter = OptionParameter::sig;
        return is;
    } else if (s == "r"){
        parameter = OptionParameter::r;
        return is;
    } else if (s == "S"){
        parameter = OptionParameter::S;
        return is;
    } else if (s == "b"){
        parameter = OptionParameter::b;
        return is;
    }
    throw std::invalid_argument("Operator>>| invalid option parameter string");
}

}// namespace option_pricing_lib
