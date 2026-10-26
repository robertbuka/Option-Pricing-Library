//
//  AmericanOption.cpp
//  option_pricing_lib
//
//  implementation of AmericanOption class functionality


#include "AmericanOption.hpp"
#include "BinomialEngine.hpp"

#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>

namespace option_pricing_lib
{

const std::vector<OptionParameter> AmericanOption::label_parameters = {OptionParameter::T, OptionParameter::K, OptionParameter::sig, OptionParameter::r,OptionParameter::S, OptionParameter::b};

AmericanOption::AmericanOption(): Option(), m_T(1) {} // default constructor, positive parameters are set to 1

AmericanOption::AmericanOption(const AmericanOption& reference): Option(reference), m_T(reference.m_T), m_binomial_number_of_steps(reference.m_binomial_number_of_steps) {} // copy constructor

AmericanOption::AmericanOption(double T, double K, double sig, double r, double S, double b): Option(K, sig, r, S, b), m_T(T) // custom constructor
{
    // check for invalid arguments
    if (T <= 0) throw std::invalid_argument("AmericanOption| Parametric constructor: T must be > 0");
}

AmericanOption::AmericanOption(const std::vector<double>& properties_array): Option()
{// custom constructor with vector as input
    // note that values in properties_array must be in the following order:
    // T, K, sig, r, S, b
    
    // check if input array has appropriate size
    if (properties_array.size() != 6)
    {
        throw std::invalid_argument("AmericanOption| Array constructor: input array size must be equal to 6");
    }
    
    // check for invalid arguments
    if (properties_array[0] <= 0 || properties_array[1] <= 0 || properties_array[2] <= 0 || properties_array[4] <= 0) throw std::invalid_argument("AmericanOption| Array constructor: T, K, sig, S must be > 0");
    
    m_T = properties_array[0];
    K(properties_array[1]);
    sig(properties_array[2]);
    r(properties_array[3]);
    S(properties_array[4]);
    b(properties_array[5]);
}

AmericanOption::~AmericanOption() {} // destructor

double AmericanOption::T() const
{// return maturity
    return m_T;
}

void AmericanOption::T(double value)
{// set maturity to value
    
    // check for invalid argument
    if (value <= 0) throw std::invalid_argument("AmericanOption| Setter: T must be > 0");
    
    m_T = value;
}

AmericanOption& AmericanOption::operator = (const AmericanOption& reference)
{// assignment operator
    
    if (&reference == this) return *this; // check for self assignment
    
    Option::operator=(reference);
    m_T = reference.m_T;
    m_binomial_number_of_steps = reference.m_binomial_number_of_steps;
    
    return *this;
}

void AmericanOption::SetParameter(OptionParameter parameter, double value)
{// set selected parameter to value
    switch (parameter)
    {
        case OptionParameter::T:
            if (value <= 0) throw std::invalid_argument("AmericanOption| Setter: T must be > 0");
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
            throw std::invalid_argument("AmericanOption| Setter: invalid option parameter");
            break;
    }
}

double AmericanOption::GetParameter(OptionParameter parameter) const
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
    throw std::invalid_argument("AmericanOption| GetParameter: invalid parameter");
}

void AmericanOption::Binomial_NSteps(long n)
{
    if (n < 3) throw std::invalid_argument("AmericanOption| Binomial steps setter: N must be > 2");
    m_binomial_number_of_steps = n;
}

long AmericanOption::Binomial_NSteps() const
{
    return m_binomial_number_of_steps;
}

double AmericanOption::CallPrice() const
{// return price of a call option
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Price(OptionTypeParameter::Call);
}

double AmericanOption::PutPrice() const
{// return price of a put option
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Price(OptionTypeParameter::Put);
}

double AmericanOption::CallPrice(double input_S) const
{// return price of a call option using input underlying value
    AmericanOption internal_option(*this);
    internal_option.S(input_S);
    
    BinomialEngine<AmericanOption> american_crr(internal_option, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Price(OptionTypeParameter::Call);
}

double AmericanOption::PutPrice(double input_S) const
{// return price of a put option using input underlying value
    AmericanOption internal_option(*this);
    internal_option.S(input_S);
    
    BinomialEngine<AmericanOption> american_crr(internal_option, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Price(OptionTypeParameter::Put);
}

double AmericanOption::CallDelta() const
{// return call delta
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Delta(OptionTypeParameter::Call);
}

double AmericanOption::PutDelta() const
{// return put delta
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Delta(OptionTypeParameter::Put);
}

double AmericanOption::CallGamma() const
{// return call gamma
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Gamma(OptionTypeParameter::Call);
}

double AmericanOption::PutGamma() const
{// return put gamma
    BinomialEngine<AmericanOption> american_crr(*this, CRR(), m_binomial_number_of_steps);
    
    return american_crr.Gamma(OptionTypeParameter::Put);
}

double AmericanOption::CallVega() const
{// return call vega
    
    if (sig() <= m_dsig) throw std::invalid_argument("AmericanOption| Call Vega: sig cannot be <= dsig");
    
    AmericanOption internal_option(*this);
    
    internal_option.SetParameter(OptionParameter::sig, this->sig() + m_dsig);
    double price_plus_dsig = internal_option.CallPrice();
    
    internal_option.SetParameter(OptionParameter::sig, this->sig() - m_dsig);
    double price_minus_dsig = internal_option.CallPrice();
    
    return (price_plus_dsig - price_minus_dsig)/(2 * m_dsig);
}

double AmericanOption::PutVega() const
{// return put vega
    
    if (sig() <= m_dsig) throw std::invalid_argument("AmericanOption| Put Vega: sig cannot be <= dsig");
    
    AmericanOption internal_option(*this);
    
    internal_option.SetParameter(OptionParameter::sig, this->sig() + m_dsig);
    double price_plus_dsig = internal_option.PutPrice();
    
    internal_option.SetParameter(OptionParameter::sig, this->sig() - m_dsig);
    double price_minus_dsig = internal_option.PutPrice();
    
    return (price_plus_dsig - price_minus_dsig)/(2 * m_dsig);
}

double AmericanOption::CallTheta() const
{// return call theta
    
    if (T() <= m_dt) throw std::invalid_argument("AmericanOption| Call Theta: T cannot be <= dt");
    
    AmericanOption internal_option(*this);
    
    internal_option.T(m_T + m_dt);
    double price_plus_dt = internal_option.CallPrice();
    
    internal_option.T(m_T - m_dt);
    double price_minus_dt = internal_option.CallPrice();
    
    return -(price_plus_dt - price_minus_dt)/(2 * m_dt);
}

double AmericanOption::PutTheta() const
{// return put theta
    
    if (T() <= m_dt) throw std::invalid_argument("AmericanOption| Put Theta: T cannot be <= dt");
    
    AmericanOption internal_option(*this);
    
    internal_option.T(m_T + m_dt);
    double price_plus_dt = internal_option.PutPrice();
    
    internal_option.T(m_T - m_dt);
    double price_minus_dt = internal_option.PutPrice();
    
    return -(price_plus_dt - price_minus_dt)/(2 * m_dt);
}

std::unique_ptr<Option> AmericanOption::clone() const
{// clone function used for vector pricers
    std::unique_ptr<Option> uptr = std::make_unique<AmericanOption>(*this);
    return uptr;
}

}
