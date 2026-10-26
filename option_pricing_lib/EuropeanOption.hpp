//
//  EuropeanOption.hpp
//  option_pricing_lib
//
//  header file for EuropeanOption class

#ifndef EuropeanOption_hpp
#define EuropeanOption_hpp

#include "Option.hpp"

#include <vector>
#include <memory>

namespace option_pricing_lib
{

class EuropeanOption: public Option
{
private:
    
    double m_T; // maturity (in years)
    double d1_helper(double input_S) const; // helper calculating d1 for given S
    double d2_helper(double input_S) const; // helper calculating d2 for given S
    
public:
    
    // option parameters
    static const std::vector<OptionParameter> label_parameters; // column labels
    static const bool allow_early_exercise = false;
    
    // for correct vector overloads
    using Option::CallPrice, Option::PutPrice, Option::CallDelta, Option::PutDelta,
    Option::CallGamma, Option::PutGamma, Option::CallVega, Option::PutVega, Option::CallTheta, Option::PutTheta;
    
    // constructors, destructor
    
    EuropeanOption(); // default constructor
    EuropeanOption(const EuropeanOption& reference); // copy constructor
    EuropeanOption(double T, double K, double sig, double r, double S, double b); // custom constructor with input parameters
    EuropeanOption(const std::vector<double>& properties_array); // custom constructor with input parameters in a form of std::vector<double>
    
    ~EuropeanOption(); // destructor
    
    // selectors
    
    std::unique_ptr<Option> clone() const override; // clone function used for vector pricers
    
    double T() const; // return maturity
    
    double GetParameter(OptionParameter parameter) const override; // return value of selected parameter
    
    // put-call parity functions
    double PutCallParity(double value, OptionTypeParameter option_parameter) const; // return call (or put) price for input put (or call) price
    bool PutCallParity(double value_1, double value_2, double tolerance = 1e-10) const; // check if a set of call/put prices satisfy parity
    
    // pricers
    
    // using current parameters
    double CallPrice() const override; // return price of a call option using current parameters
    double PutPrice() const override; // return price of a put option using current parameters
    
    // using input_S
    double CallPrice(double input_S) const override; // return price of a call option using input underlying value
    double PutPrice(double input_S) const override; // return price of a put option using input underlying value
    
    // sensitivities
    // analytical
    
    // using current object properties
    double CallDelta() const override; // return call delta
    double PutDelta() const override; // return put delta
    
    // note that for EuropeanOption, call gamma == put gamma, call vega == put vega
    // functions are separated for compatibility purposes
    
    double CallGamma() const override; // return call gamma
    double PutGamma () const override; // return put gamma
    
    double CallVega() const override; // return call vega
    double PutVega() const override; // return put vega
    
    double CallTheta() const override; // return call theta
    double PutTheta() const override; // return put theta
    
    // modifiers
    
    void T(double value); // set m_T to value
    
    void SetParameter(OptionParameter parameter, double value) override; // set selected parameter to value
    
    // operator overloading
    
    EuropeanOption& operator = (const EuropeanOption& reference); // assignment operator
};
}// namespace option_pricing_lib

#endif /* EuropeanOption_hpp */
