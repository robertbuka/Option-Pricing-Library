//
//  BinomialStrategy.hpp
//  option_pricing_lib
//
//  header file for abstract base class BinomialStrategy
//  and derived classes (Cox-Ross-Rubinstein, Jarrow-Rudd)

#ifndef BinomialStrategy_hpp
#define BinomialStrategy_hpp

#include <memory>

namespace option_pricing_lib
{

class BinomialStrategy
{
public:
    
    // destructor
    
    virtual ~BinomialStrategy() {}
    
    // selectors
    
    virtual double u(double sig, double b, double dt) const = 0;
    virtual double d(double sig, double b, double dt) const = 0;
    virtual double p(double sig, double b, double dt) const = 0;
    
    // clone
    
    virtual std::unique_ptr<BinomialStrategy> clone() const = 0;
    
};

class CRR: public BinomialStrategy
{
    
public:
    
    // selectors
    
    double u(double sig, double b, double dt) const override;
    double d(double sig, double b, double dt) const override;
    double p(double sig, double b, double dt) const override;
    
    // clone
    
    std::unique_ptr<BinomialStrategy> clone() const override;
};

class JR: public BinomialStrategy
{
    
public:
    
    // selectors
    
    double u(double sig, double b, double dt) const override;
    double d(double sig, double b, double dt) const override;
    double p(double sig, double b, double dt) const override;
    
    // clone
    
    std::unique_ptr<BinomialStrategy> clone() const override;
};
} // namespace option_pricing_lib

#endif /* BinomialStrategy_hpp */
