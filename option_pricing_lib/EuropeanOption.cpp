//
//  EuropeanOption.cpp
//  option_pricing_lib
//
//  implementation of EuropeanOption class functionality

#include "EuropeanOption.hpp"
#include <boost/math/distributions/normal.hpp>


#include <algorithm>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace option_pricing_lib
{

const std::vector<OptionParameter> EuropeanOption::label_parameters = {OptionParameter::T, OptionParameter::K, OptionParameter::sig, OptionParameter::r,OptionParameter::S, OptionParameter::b};

EuropeanOption::EuropeanOption(): Option(), m_T(1) {} // default constructor, positive parameters are set to 1

EuropeanOption::EuropeanOption(const EuropeanOption& reference): Option(reference), m_T(reference.m_T) {} // copy constructor

EuropeanOption::EuropeanOption(double T, double K, double sig, double r, double S, double b): Option(K, sig, r, S, b), m_T(T) // custom constructor
{
    // check for invalid arguments
    if (T <= 0) throw std::invalid_argument("EuropeanOption| Parametric constructor: T must be > 0");
}

EuropeanOption::EuropeanOption(const std::vector<double>& properties_array): Option()
{// custom constructor with vector as input
    // note that values in properties_array must be in the following order:
    // T, K, sig, r, S, b
    
    // check if input array has appropriate size
    if (properties_array.size() != 6)
    {
        throw std::invalid_argument("EuropeanOption| Array constructor: input array size must be equal to 6");
    }
    
    // check for invalid arguments
    if (properties_array[0] <= 0 || properties_array[1] <= 0 || properties_array[2] <= 0 || properties_array[4] <= 0) throw std::invalid_argument("EuropeanOption| Array constructor: T, K, sig, S must be > 0");
    
    m_T = properties_array[0];
    K(properties_array[1]);
    sig(properties_array[2]);
    r(properties_array[3]);
    S(properties_array[4]);
    b(properties_array[5]);
}

EuropeanOption::~EuropeanOption() {} // destructor

double EuropeanOption::T() const
{// return maturity
    return m_T;
}

void EuropeanOption::T(double value)
{// set maturity to value
    
    // check for invalid argument
    if (value <= 0) throw std::invalid_argument("EuropeanOption| Setter: T must be > 0");
    
    m_T = value;
}

double EuropeanOption::CallPrice() const
{// calculate price of a call option
    return CallPrice(S());
}

double EuropeanOption::PutPrice() const
{// calculate price of a put option
    return PutPrice(S());
}

EuropeanOption& EuropeanOption::operator = (const EuropeanOption& reference)
{// assignment operator
    
    if (&reference == this) return *this; // check for self assignment
    
    Option::operator=(reference);
    m_T = reference.m_T;
    
    return *this;
}

double EuropeanOption::CallPrice(double input_S) const
{// return price of a call option using input underlying value
    
    // check for invalid input
    if (input_S <= 0) throw std::invalid_argument("EuropeanOption| S must be greater than 0");
    
    // access data from base class
    double K = Option::K();
    double r = Option::r();
    double b = Option::b();
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    //using helpers
    double d1 = d1_helper(input_S);
    double d2 = d2_helper(input_S);
    
    double price = input_S * std::exp((b - r) * m_T) * boost::math::cdf(normal, d1) - K * std::exp(-r * m_T) * boost::math::cdf(normal, d2); // Black-Scholes model
    
    return price;
}

double EuropeanOption::PutPrice(double input_S) const
{// return price of a put option using input underlying value
    
    // check for invalid input
    if (input_S <= 0) throw std::invalid_argument("EuropeanOption| S must be greater than 0");
    
    // access data from base class
    double K = Option::K();
    double r = Option::r();
    double b = Option::b();
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    //using helpers
    double d1 = d1_helper(input_S);
    double d2 = d2_helper(input_S);
    
    double price = K * std::exp(-r * m_T) * boost::math::cdf(normal, -d2) - input_S * std::exp((b - r) * m_T) * boost::math::cdf(normal, -d1); // Black-Scholes model
    
    return price;
}

double EuropeanOption::PutCallParity(double value, OptionTypeParameter option_parameter) const
{// calculate call (or put) for a corresponding put (or call)
    // note that option_parameter can only be OptionTypeParameter::Call or OptionTypeParameter::Put
    // if input is OptionTypeParameter::Call, output is put (and vice versa)
    
    // access data from base class
    double K = Option::K();
    double r = Option::r();
    double S = Option::S();
    double b = Option::b();
    
    double price;
    
    // different formula depending on option_parameter
    switch (option_parameter)
    {
        case OptionTypeParameter::Call:
            return price = value + K * std::exp(-r * m_T) - S * std::exp((b - r) * m_T);
            break;
            
        case OptionTypeParameter::Put:
            return price = value + S * std::exp((b - r) * m_T) - K * std::exp(-r * m_T);
            break;
        default:
            throw std::invalid_argument("EuropeanOption| P-C parity: invalid option parameter");
    }
}

bool EuropeanOption::PutCallParity(double value_1, double value_2, double tolerance) const
{// check if a set of call/put prices satisfies parity
    // as value_1 and value_2 are not specified to be call / put, check two permutations
    // note that tolerance scales for larger values
    
    // check for invalid input
    if (value_1 < 0 || value_2 < 0 || tolerance < 0) throw std::invalid_argument("EuropeanOption| P-C parity: both values and tolerance must be non negative");
    
    // access data from base class
    double K = Option::K();
    double r = Option::r();
    double S = Option::S();
    double b = Option::b();
    
    // two sides of put call parity equation
    double left_hand_side = value_1 + K * std::exp((-r) * m_T);
    double right_hand_side = value_2 + S * std::exp((b - r) * m_T);
    
    bool satisfy_parity_permutation_1 = std::abs(left_hand_side - right_hand_side) < tolerance * std::max({1.0,std::abs(left_hand_side),std::abs(right_hand_side)}); // check against scaled tolerance
    
    // check for the second permutation
    left_hand_side = value_2 + K * std::exp((-r) * m_T);
    right_hand_side = value_1 + S * std::exp((b - r) * m_T);
    
    bool satisfy_parity_permutation_2 = std::abs(left_hand_side - right_hand_side) < tolerance * std::max({1.0,std::abs(left_hand_side),std::abs(right_hand_side)}); // check against scaled tolerance
    
    return satisfy_parity_permutation_1 || satisfy_parity_permutation_2;
}

double EuropeanOption::CallDelta() const
{// calculate and return call delta
    // access data from base class
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    double d1 = d1_helper(S);
    
    double output = std::exp((b - r) * m_T) * boost::math::cdf(normal, d1); // call delta formula
    
    return output;
}

double EuropeanOption::PutDelta() const
{// calculate and return put delta
    // access data from base class
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    double d1 = d1_helper(S);
    
    double output = std::exp((b - r) * m_T) * (boost::math::cdf(normal, d1) - 1); // put delta formula
    
    return output;
}

double EuropeanOption::CallGamma() const
{// calculate and return call gamma
    // note that gamma formula is same for call/put
    
    // access data from base class
    double sig = Option::sig();
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    double d1 = d1_helper(S);
    
    double output = (boost::math::pdf(normal, d1) * std::exp((b - r) * m_T)) / (S * sig * std::sqrt(m_T)); // gamma formula
    
    return output;
}

double EuropeanOption::PutGamma() const
{// for compatibility purposes
    // note that gamma formula is same for call/put
    
    return CallGamma();
}

void EuropeanOption::SetParameter(OptionParameter parameter, double value)
{// set selected parameter to value
    switch (parameter)
    {
        case OptionParameter::T:
            if (value <= 0) throw std::invalid_argument("EuropeanOption| Setter: T must be > 0");
            m_T = value;
            break;
        case OptionParameter::K:
            K(value);
            break;
        case OptionParameter::sig:
            sig(value);
            break;
        case OptionParameter::r:
            r(value);
            break;
        case OptionParameter::S:
            S(value);
            break;
        case OptionParameter::b:
            b(value);
            break;
        default:
            throw std::invalid_argument("EuropeanOption| Setter: invalid option parameter");
            break;
    }
}

double EuropeanOption::d1_helper(double input_S) const
{// helper calculating d1
    return (std::log(input_S / K()) + (b() + (sig() * sig()) / 2.0) * m_T) / (sig() * std::sqrt(m_T));
}

double EuropeanOption::d2_helper(double input_S) const
{// helper calculating d2
    return (std::log(input_S / K()) + (b() - (sig() * sig()) / 2.0) * m_T) / (sig() * std::sqrt(m_T));
}

std::unique_ptr<Option> EuropeanOption::clone() const
{
    // create unique pointer to a copy of current object
    std::unique_ptr<Option> ptr = std::make_unique<EuropeanOption>(*this);
    return ptr;
}

double EuropeanOption::CallVega() const
{// return call vega
    // note that vega formula is same for call/put
    
    // access data from base class
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    
    double d1 = d1_helper(S);
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    return S * std::exp((b-r) * m_T) * boost::math::pdf(normal, d1) * std::sqrt(m_T);
}

double EuropeanOption::PutVega() const
{// return put vega
    // note that vega formula is same for call/put
    return CallVega();
}

double EuropeanOption::CallTheta() const
{// return call theta
    // access data from base class
    double sig = Option::sig();
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    double K = Option::K();
    
    double d1 = d1_helper(S);
    double d2 = d2_helper(S);
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    return -((S * sig * std::exp((b-r) * m_T)) * boost::math::pdf(normal, d1)) / (2 * std::sqrt(m_T)) - (b - r) * S * std::exp((b-r) * m_T) * boost::math::cdf(normal, d1) - r * K * std::exp(-r * m_T) * boost::math::cdf(normal, d2);
}

double EuropeanOption::PutTheta() const
{// return put theta
    // access data from base class
    double sig = Option::sig();
    double r = Option::r();
    double b = Option::b();
    double S = Option::S();
    double K = Option::K();
    
    double d1 = d1_helper(S);
    double d2 = d2_helper(S);
    
    boost::math::normal_distribution<> normal(0.0, 1.0); // create standard normal distribution
    
    return -((S * sig * std::exp((b-r) * m_T)) * boost::math::pdf(normal, d1)) / (2 * std::sqrt(m_T)) + (b - r) * S * std::exp((b-r) * m_T) * boost::math::cdf(normal, -d1) + r * K * std::exp(-r * m_T) * boost::math::cdf(normal, -d2);
}

double EuropeanOption::GetParameter(OptionParameter parameter) const
{
    switch (parameter)
    {
        case OptionParameter::T:
            return m_T;
            break;
        case OptionParameter::K:
            return K();
            break;
        case OptionParameter::sig:
            return sig();
            break;
        case OptionParameter::r:
            return r();
            break;
        case OptionParameter::S:
            return S();
            break;
        case OptionParameter::b:
            return b();
            break;
    }
    throw std::invalid_argument("EuropeanOption| GetParameter: invalid parameter");
}

}// namespace option_pricing_lib
