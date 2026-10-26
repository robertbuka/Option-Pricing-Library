//
//  AmericanPerpetualOption.hpp
//  option_pricing_lib
//
//  header file for AmericanPerpetualOption class

#ifndef AmericanPerpetualOption_hpp
#define AmericanPerpetualOption_hpp

#include "Option.hpp"

#include <vector>
#include <cmath>

namespace option_pricing_lib
{

class AmericanPerpetualOption: public Option
{
private:
    
    double m_y1() const // y1 helper function
    {
        return 0.5 - (b() / (sig() * sig())) + std::sqrt(std::pow(((b() / (sig() * sig())) - 0.5), 2) + 2 * r() / std::pow(sig(), 2));
    }
    
    double m_y2() const // y2 helper function
    {
        return 0.5 - (b() / (sig() * sig())) - std::sqrt(std::pow(((b() / (sig() * sig())) - 0.5), 2) + 2 * r() / std::pow(sig(), 2));
    }
    
public:
    
    // option parameters
    static const std::vector<OptionParameter> label_parameters; // column labels for matrix pricer
    static const bool allow_early_exercise = true;
    
    // for correct vector overloads
    using Option::CallPrice, Option::PutPrice, Option::CallDelta, Option::PutDelta,
    Option::CallGamma, Option::PutGamma, Option::CallVega, Option::PutVega, Option::CallTheta, Option::PutTheta;
    
    // constructors, destructor
    
    AmericanPerpetualOption(); // default constructor
    AmericanPerpetualOption(const AmericanPerpetualOption& reference); // copy constructor
    AmericanPerpetualOption(double K, double sig, double r, double S, double b); // custom constructor with input parameters
    AmericanPerpetualOption(const std::vector<double>& properties_array); // custom constructor with input parameters in a form of std::vector<double>
    
    virtual ~AmericanPerpetualOption(); // destructor
    
    // selectors
    
    std::unique_ptr<Option> clone() const override; // clone function used for vector pricers
    
    double GetParameter(OptionParameter parameter) const override; // return value of selected parameter
    
    // pricers
    
    // using current parameters
    double CallPrice() const override; // return price of a call option
    double PutPrice() const override; // return price of a put option
    
    // using input_S
    double CallPrice(double input_S) const override; // return price of a call option using input underlying value
    double PutPrice(double input_S) const override; // return price of a put option using input underlying value
    
    // sensitivities
    // analytical
    
    double CallDelta() const override; // return call delta
    double PutDelta() const override; // return put delta
    
    double CallGamma() const override; // return call gamma
    double PutGamma() const override; // return put gamma
    
    double CallVega() const override; // return call vega
    double PutVega() const override; // return put vega
    
    double CallTheta() const override; // return call theta
    double PutTheta() const override; // return put theta
    
    // critical prices
    
    double CriticalPriceCall() const; // critical price for call
    double CriticalPricePut() const; // critical price for put
    
    // modifiers
    
    void SetParameter(OptionParameter parameter, double value) override; // set selected parameter to value
    
    // operator overloading
    
    AmericanPerpetualOption& operator = (const AmericanPerpetualOption& reference); // assignment operator
};
}// namespace option_pricing_lib

#endif /* AmericanPerpetualOption_hpp */
