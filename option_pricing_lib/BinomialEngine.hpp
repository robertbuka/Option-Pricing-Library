//
//  BinomialEngine.hpp
//  option_pricing_lib
//
//  header file for BinomialEngine class template

#ifndef BinomialEngine_hpp
#define BinomialEngine_hpp

#include "BinomialStrategy.hpp"
#include "Option.hpp"
#include "AmericanPerpetualOption.hpp"
#include <memory>
#include <map>
#include <type_traits>

namespace option_pricing_lib
{

template <typename OptionType>
class BinomialEngine
{
    // check that T exists
    static_assert(!std::is_same_v<OptionType, AmericanPerpetualOption>, "BinomialEngine| Option type must have an expiry");
    
    OptionType m_internal_option;
    std::unique_ptr<BinomialStrategy> m_strategy;
    long m_N; // number of steps
    
    struct TreeValues
    {
        double price;
        double delta;
        double gamma;
    };
    
    // private helper
    TreeValues Tree(OptionTypeParameter parameter) const;
    
public:
    
    // constructors, destructor
    
    BinomialEngine(const OptionType& internal_option, const BinomialStrategy& strategy, long N);
    BinomialEngine(const BinomialEngine<OptionType>& reference);
    
    ~BinomialEngine();
    
    // operators
    
    BinomialEngine<OptionType>& operator = (const BinomialEngine<OptionType>& reference);
    
    // modifiers
    
    void N(long n);
    
    void Option(const OptionType& reference);
    
    void Strategy(const BinomialStrategy& reference);
    
    // selectors
    
    long N() const;
    
    const OptionType& Option() const;
    
    // pricer and sensitivities
    
    double Price(OptionTypeParameter parameter) const;
    double Delta(OptionTypeParameter parameter) const;
    double Gamma(OptionTypeParameter parameter) const;
    
};
} // namespace option_pricing_lab

#ifndef BinomialEngine_cpp
#include "BinomialEngine.cpp"
#endif

#endif /* BinomialEngine_hpp */
