//
//  AmericanOption.hpp
//  option_pricing_lib
//
//  header file for AmericanOption Class

#ifndef AmericanOption_hpp
#define AmericanOption_hpp

#include "Option.hpp"

#include <vector>
#include <memory>

namespace option_pricing_lib
{

class AmericanOption: public Option
{
    double m_T; // maturity (in years)
    long m_binomial_number_of_steps = 1000;
    
    // small change in sig and t
    // used for calculating vega and theta
    static constexpr double m_dsig = 1e-4;
    static constexpr double m_dt = 1e-4;
    
public:
    
    // option parameters
    static const std::vector<OptionParameter> label_parameters; // column labels
    static const bool allow_early_exercise = true;
    
    // for correct vector overloads
    using Option::CallPrice, Option::PutPrice, Option::CallDelta, Option::PutDelta,
    Option::CallGamma, Option::PutGamma, Option::CallVega, Option::PutVega, Option::CallTheta, Option::PutTheta;
    
    // constructors, destructor
    
    AmericanOption();
    AmericanOption(const AmericanOption& reference);
    AmericanOption(double T, double K, double sig, double r, double S, double b); // custom constructor with input parameters
    AmericanOption(const std::vector<double>& properties_array); // custom constructor with input parameters in a form of std::vector<double>
    
    ~AmericanOption();
    
    // selectors
    
    std::unique_ptr<Option> clone() const override; // clone function used for vector pricers
    
    double GetParameter(OptionParameter parameter) const override; // return value of selected parameter
    
    double T() const; // return maturity
    
    long Binomial_NSteps() const;
    
    // using current object properties
    double CallPrice() const override; // return price of a call option
    double PutPrice() const override; // return price of a put option
    
    // using input_S
    double CallPrice(double input_S) const override; // return price of a call option using input underlying value
    double PutPrice(double input_S) const override; // return price of a put option using input underlying value
    
    // sensitivities
    
    double CallDelta() const override; // return call delta
    double PutDelta() const override; // return put delta
    
    double CallGamma() const override; // return call gamma
    double PutGamma() const override; // return put gamma
    
    double CallVega() const override; // return call vega
    double PutVega() const override; // return put vega
    
    double CallTheta() const override; // return call theta
    double PutTheta() const override; // return put theta
    
    // modifiers
    
    void SetParameter(OptionParameter parameter, double value) override; // set selected parameter to value
    
    void T(double T);
    
    void Binomial_NSteps(long n);
    
    // operators
    
    AmericanOption& operator = (const AmericanOption& reference); // assignment operator
    
};
}// namespace option_pricing_lib

#endif /* AmericanOption_hpp */
