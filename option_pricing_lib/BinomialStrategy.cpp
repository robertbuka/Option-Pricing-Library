//
//  BinomialStrategy.cpp
//  option_pricing_lib
//
//  implementation of BinomialStrategy derived classes functionality

#include "BinomialStrategy.hpp"
#include <cmath>
#include <memory>

namespace option_pricing_lib
{

double CRR::u(double sig, double, double dt) const
{
    return std::exp(sig * std::sqrt(dt));
}

double CRR::d(double sig, double, double dt) const
{
    return std::exp(-sig * std::sqrt(dt));
}

double CRR::p(double sig, double b, double dt) const
{
    return (std::exp(b*dt)-d(sig, b, dt))/(u(sig, b, dt) - d(sig, b, dt));
}

double JR::u(double sig, double b, double dt) const
{
    return std::exp((b - 0.5 * sig * sig) * dt + sig * std::sqrt(dt));
}

double JR::d(double sig, double b, double dt) const
{
    return std::exp((b - 0.5 * sig * sig) * dt - sig * std::sqrt(dt));
}

double JR::p(double, double, double) const
{
    return 0.5;
}

std::unique_ptr<BinomialStrategy> CRR::clone() const
{
    std::unique_ptr<BinomialStrategy> uptr = std::make_unique<CRR>(*this);
    
    return uptr;
}

std::unique_ptr<BinomialStrategy> JR::clone() const
{
    std::unique_ptr<BinomialStrategy> uptr = std::make_unique<JR>(*this);
    
    return uptr;
}

}// namespace option_pricing_lib
