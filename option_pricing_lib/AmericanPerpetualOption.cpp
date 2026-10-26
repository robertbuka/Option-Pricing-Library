//
//  AmericanPerpetualOption.cpp
//  option_pricing_lib
//
//  implementation of AmericanPerpetualOption class functionality

#include "AmericanPerpetualOption.hpp"

#include <vector>
#include <cmath>
#include <stdexcept>

namespace option_pricing_lib
{

const std::vector<OptionParameter> AmericanPerpetualOption::label_parameters = {OptionParameter::K, OptionParameter::sig, OptionParameter::r,OptionParameter::S, OptionParameter::b};

AmericanPerpetualOption::AmericanPerpetualOption(): Option(100, 0.1, 0.1, 110, 0.02) {} // default constructor
// values set in order to make
// both call and put valid

AmericanPerpetualOption::AmericanPerpetualOption(const AmericanPerpetualOption& reference): Option(reference) {} // copy constructor

AmericanPerpetualOption::AmericanPerpetualOption(double K, double sig, double r, double S, double b): Option(K, sig, r, S, b) {} // custom constructor with input parameters

AmericanPerpetualOption::AmericanPerpetualOption(const std::vector<double>& properties_array): Option(properties_array) {} // custom constructor with input parameters in a form of std::vector<double>

AmericanPerpetualOption::~AmericanPerpetualOption() {} // destructor

double AmericanPerpetualOption::CallPrice(double input_S) const
{// return price of a call option
    
    // check for invalid input
    if (b() >= r()) throw std::invalid_argument("AmericanPerpetualOption| b must be less than r for call");
    if (input_S <= 0) throw std::invalid_argument("AmericanPerpetualOption| S must be greater than 0");
    
    // access base class data
    double K = Option::K();
    double S = input_S;
    double y1 = m_y1();
    
    // check critical price boundary
    if (S >= CriticalPriceCall()) return S - K;
    
    // find price using formula for perpetual American option
    double price = (K / (y1 - 1)) * std::pow(((y1 - 1)/y1) * S / K, y1);
    
    return price;
}

double AmericanPerpetualOption::PutPrice(double input_S) const
{// return price of a put option
    
    // check for invalid input
    if (r() <= 0) throw std::invalid_argument("AmericanPerpetualOption| r must be greater than 0 for put");
    if (input_S <= 0) throw std::invalid_argument("AmericanPerpetualOption| S must be greater than 0");
    
    // access base class data
    double K = Option::K();
    double S = input_S;
    double y2 = m_y2();
    
    // check critical price boundary
    if (S <= CriticalPricePut()) return K - S;
    
    // find price using formula for perpetual American option
    double price = (K / (1 - y2)) * std::pow(((y2 - 1)/y2) * S / K, y2);
    
    return price;
}

double AmericanPerpetualOption::CallPrice() const
{// return price of a call option
    
    // use CallPrice(input_S) with Option::S()
    return CallPrice(S());
}

double AmericanPerpetualOption::PutPrice() const
{// return price of a put option
    
    // use PutPrice(input_S) with Option::S()
    return PutPrice(S());
}

AmericanPerpetualOption& AmericanPerpetualOption::operator = (const AmericanPerpetualOption &reference)
{// assignment operator
    if (&reference == this) return *this;
    
    Option::operator=(reference);
    
    return *this;
}

void AmericanPerpetualOption::SetParameter(OptionParameter parameter, double value)
{// set selected parameter to value
    switch (parameter)
    {
        case OptionParameter::T:
            throw std::invalid_argument("AmericanPerpetualOption| Setter: this type does not have T");
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
            throw std::invalid_argument("AmericanPerpetualOption| Setter: invalid option parameter");
            break;
    }
}

// following functions are for compatibility purposes
// h set to value achieving best approximation

double AmericanPerpetualOption::CallDelta() const
{// return call delta
    
    // call pricer to check validity
    double price = CallPrice();
    
    // check critical price boundary
    if (S() >= CriticalPriceCall()) return 1;
    
    return m_y1() * price / S();
}

double AmericanPerpetualOption::PutDelta() const
{// return put delta
    
    // call pricer to check validity
    double price = PutPrice();
    
    // check critical price boundary
    if (S() <= CriticalPricePut()) return -1;
    
    return m_y2() * price / S();
}

double AmericanPerpetualOption::CallGamma() const
{// return call gamma
    
    // call pricer to check validity
    double price = CallPrice();
    
    // check critical price boundary
    if (S() >= CriticalPriceCall()) return 0;
    
    return m_y1() * (m_y1() - 1) * price / (S() * S());
}

double AmericanPerpetualOption::PutGamma() const
{// return put gamma
    
    // call pricer to check validity
    double price = PutPrice();
    
    // check critical price boundary
    if (S() <= CriticalPricePut()) return 0;
    
    return m_y2() * (m_y2() - 1) * price / (S() * S());
}

double AmericanPerpetualOption::CriticalPriceCall() const
{// critical price for call
    // check for invalid input
    if (b() >= r()) throw std::invalid_argument("AmericanPerpetualOption| b must be less than r for call");
    
    return (K() * m_y1()) / (m_y1() - 1);
}

double AmericanPerpetualOption::CriticalPricePut() const
{// critical price for put
    // check for invalid input
    if (r() <= 0) throw std::invalid_argument("AmericanPerpetualOption| r must be greater than 0 for put");
    
    return (K() * m_y2()) / (m_y2() - 1);
}

std::unique_ptr<Option> AmericanPerpetualOption::clone() const
{
    // create unique pointer to a copy of current object
    std::unique_ptr<Option> ptr = std::make_unique<AmericanPerpetualOption>(*this);
    return ptr;
}

double AmericanPerpetualOption::CallVega() const
{// return call vega
    
    // check critical price boundary
    if (S() >= CriticalPriceCall()) return 0;
    
    // access data from base class
    double b = Option::b();
    double sig = Option::sig();
    double S = Option::S();
    
    // using y1 for call
    double y1 = m_y1();
    
    return CallPrice() * std::log(S / CriticalPriceCall()) * (-sig * y1 * (y1 - 1))/(sig * sig * (y1 - 0.5) + b);
}

double AmericanPerpetualOption::PutVega() const
{// return put vega
    
    // check critical price boundary
    if (S() <= CriticalPricePut()) return 0;
    
    // access data from base class
    double b = Option::b();
    double sig = Option::sig();
    double S = Option::S();
    
    // using y1 for call
    double y2 = m_y2();
    
    return PutPrice() * std::log(S / CriticalPricePut()) * (-sig * y2 * (y2 - 1))/(sig * sig * (y2 - 0.5) + b);
}

double AmericanPerpetualOption::CallTheta() const
{// return call theta
    // american perp does not depend on time
    // check for invalid input
    if (b() >= r()) throw std::invalid_argument("AmericanPerpetualOption| b must be less than r for call");
    
    return 0;
}

double AmericanPerpetualOption::PutTheta() const
{// return put theta
    // american perp does not depend on time
    // check for invalid input
    if (r() <= 0) throw std::invalid_argument("AmericanPerpetualOption| r must be greater than 0");
    
    return 0;
}

double AmericanPerpetualOption::GetParameter(OptionParameter parameter) const
{
    switch (parameter)
    {
        case OptionParameter::T:
            throw std::invalid_argument("AmericanPerpetualOption| GetParameter: T is not supported");
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
    throw std::invalid_argument("AmericanPerpetualOption| GetParameter: invalid parameter");
}

}// namespace option_pricing_lib
