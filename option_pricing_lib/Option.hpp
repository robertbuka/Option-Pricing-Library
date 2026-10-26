//
//  Option.hpp
//  option_pricing_lib
//
//  header file for Option abstract base class and supporting enum classes

#ifndef Option_hpp
#define Option_hpp

#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

namespace option_pricing_lib
{
enum class OptionTypeParameter
{// choose option type
    Call,
    Put
};

enum class OptionParameter
{// choose varying parameter for vector pricers and sensitivities, matrix pricers and SetParameter
    T,
    K,
    sig,
    r,
    S,
    b
};

// operator overloads for reading/writing labels in matrix pricers
std::ostream& operator << (std::ostream& os, OptionParameter parameter);
std::istream& operator >> (std::istream& is, OptionParameter& parameter);

class Option
{
private:
    
    // option parameters
    double m_K; // strike price
    double m_sig; // volatility
    double m_r; // risk-free interest rate
    double m_S; // current stock price
    double m_b; // cost of carry
    
    // divided differences helper
    std::vector<double> DDHelper(const std::vector<double>& input_S, double h, const std::function<double(double,double)>& dd_func) const;
    // vector pricer helper
    std::vector<double> VPHelper(const std::vector<double>& input, OptionParameter parameter, const std::function<double(const Option&)>& vp_func) const;
    
public:
    
    // constructors, destructor
    
    Option(); // default constructor
    Option(const Option& reference); // copy constructor
    Option(double K, double sig, double r, double S, double b); // custom constructor with input parameters
    Option(const std::vector<double>& properties_array); // custom constructor with input parameters in a form of std::vector<double>
    
    virtual ~Option(); // destructor
    
    // selectors
    
    virtual std::unique_ptr<Option> clone() const = 0; // pure virtual clone function used for vector pricers
    
    // return value of a relevant member field
    double K() const;
    double sig() const;
    double r() const;
    double S() const;
    double b() const;
    
    virtual double GetParameter(OptionParameter parameter) const = 0; // return value of selected parameter
    
    // pricers
    
    // using current object properties
    virtual double CallPrice() const = 0; // return price of a call option
    virtual double PutPrice() const = 0; // return price of a put option
    
    // using input_S
    virtual double CallPrice(double input_S) const = 0; // return price of a call option using input underlying value
    virtual double PutPrice(double input_S) const = 0; // return price of a put option using input underlying value
    
    // using array of option parameter values
    std::vector<double> CallPrice(const std::vector<double>& input, OptionParameter parameter) const; // return a vector of call option prices based on input (vector of option parameter values)
    std::vector<double> PutPrice(const std::vector<double>& input, OptionParameter parameter) const; // return a vector of put option prices based on input (vector of option parameter values)
    
    // sensitivities
    
    // analytical
    // for compatibility purposes, option types with no exact solution for sensitivities may call divided differences method
    
    virtual double CallDelta() const = 0; // return call delta
    virtual double PutDelta() const = 0; // return put delta
    
    virtual double CallGamma() const = 0; // return call gamma
    virtual double PutGamma() const = 0; // return put gamma
    
    virtual double CallVega() const = 0; // return call vega
    virtual double PutVega() const = 0; // return put vega
    
    virtual double CallTheta() const = 0; // return call theta
    virtual double PutTheta() const = 0; // return put theta
    
    // using array of option parameter values
    // return a vector of call/put sensitivity based on input (vector of option parameter values)
    
    std::vector<double> CallDelta(const std::vector<double>& input, OptionParameter parameter) const;
    std::vector<double> PutDelta(const std::vector<double>& input, OptionParameter parameter) const;
    
    std::vector<double> CallGamma(const std::vector<double>& input, OptionParameter parameter) const;
    std::vector<double> PutGamma(const std::vector<double>& input, OptionParameter parameter) const;
    
    std::vector<double> CallVega(const std::vector<double>& input, OptionParameter parameter) const;
    std::vector<double> PutVega(const std::vector<double>& input, OptionParameter parameter) const;
    
    std::vector<double> CallTheta(const std::vector<double>& input, OptionParameter parameter) const;
    std::vector<double> PutTheta(const std::vector<double>& input, OptionParameter parameter) const;
    
    // divided differences method
    
    // using current object properties
    double DividedDifferencesCallDelta(double h) const; // return call delta
    double DividedDifferencesPutDelta(double h) const; // return put delta
    
    double DividedDifferencesCallGamma(double h) const; // return call gamma
    double DividedDifferencesPutGamma(double h) const; // return put gamma
    
    // using input_S
    double DividedDifferencesCallDelta(double input_S, double h) const; // return call delta using input underlying value
    double DividedDifferencesPutDelta(double input_S, double h) const; // return put delta using input underlying value
    
    double DividedDifferencesCallGamma(double input_S, double h) const; // return call gamma using input underlying value
    double DividedDifferencesPutGamma(double input_S, double h) const; // return put gamma using input underlying value
    
    // using array of input_S values
    std::vector<double> DividedDifferencesCallDelta(const std::vector<double>& input_S, double h) const; // return a vector of call delta based on input_S (vector of underlying values)
    std::vector<double> DividedDifferencesPutDelta(const std::vector<double>& input_S, double h) const; // return a vector of put delta based on input_S (vector of underlying values)
    
    std::vector<double> DividedDifferencesCallGamma(const std::vector<double>& input_S, double h) const; // return a vector of call gamma based on input_S (vector of underlying values)
    std::vector<double> DividedDifferencesPutGamma(const std::vector<double>& input_S, double h) const; // return a vector of put gamma based on input_S (vector of underlying values)
    
    
    // modifiers
    
    // modify value of a relevant member field
    void K(double value);
    void sig(double value);
    void r(double value);
    void S(double value);
    void b(double value);
    
    // set parameter to a value using OptionParameter
    
    virtual void SetParameter(OptionParameter parameter, double value) = 0; // set selected parameter to value
    
    // operator overloading
    
    Option& operator = (const Option& reference); // assignment operator
};
} // namespace option_pricing_lib

#endif /* Option_hpp */
