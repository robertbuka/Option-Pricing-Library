//
//  BinomialEngine.cpp
//  option_pricing_lib
//
//  implementation of BinomialEngine class template functionality

#ifndef BinomialEngine_cpp
#define BinomialEngine_cpp

#include "Option.hpp"
#include "BinomialEngine.hpp"

#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>

namespace option_pricing_lib
{

template <typename OptionType>
BinomialEngine<OptionType>::BinomialEngine(const OptionType& internal_option, const BinomialStrategy& strategy, long N): m_internal_option(internal_option), m_N(N)
{
    if (N < 3) throw std::invalid_argument("BinomialEngine| N must be > 2");
    m_strategy = strategy.clone();
}

template <typename OptionType>
BinomialEngine<OptionType>::BinomialEngine(const BinomialEngine& reference): m_internal_option(reference.m_internal_option), m_N(reference.m_N)
{
    m_strategy = reference.m_strategy->clone();
}

template <typename OptionType>
BinomialEngine<OptionType>::~BinomialEngine() {}

template <typename OptionType>
BinomialEngine<OptionType>& BinomialEngine<OptionType>::operator = (const BinomialEngine& reference)
{
    // check for self assignment
    if (&reference == this) return *this;
    
    m_internal_option = reference.m_internal_option;
    m_strategy = reference.m_strategy->clone();
    m_N = reference.m_N;
    
    return *this;
}

template <typename OptionType>
long BinomialEngine<OptionType>::N() const
{
    return m_N;
}

template <typename OptionType>
const OptionType& BinomialEngine<OptionType>::Option() const
{
    return m_internal_option;
}

template <typename OptionType>
void BinomialEngine<OptionType>::N(long n)
{
    if (n < 3) throw std::invalid_argument("BinomialEngine| N must be > 2");
    m_N = n;
}

template <typename OptionType>
void BinomialEngine<OptionType>::Option(const OptionType& reference)
{
    m_internal_option = reference;
}

template <typename OptionType>
void BinomialEngine<OptionType>::Strategy(const BinomialStrategy& reference)
{
    m_strategy = reference.clone();
}

template <typename OptionType>
BinomialEngine<OptionType>::TreeValues BinomialEngine<OptionType>::Tree(OptionTypeParameter parameter) const
{
    auto payoff = [](double S, double K, OptionTypeParameter parameter)
    {
        switch (parameter)
        {
            case OptionTypeParameter::Call:
                return std::max(S-K,0.0);
                break;
            case OptionTypeParameter::Put:
                return std::max(K-S,0.0);
                break;
            default:
                throw std::invalid_argument("BinomialEngine| Tree: invalid option parameter");
                break;
        }
    };
    
    // access option data
    double S = m_internal_option.S();
    double K = m_internal_option.K();
    double sig = m_internal_option.sig();
    double b = m_internal_option.b();
    double r = m_internal_option.r();
    
    // compute u, d, p, dt, discount factor
    double dt = m_internal_option.T() / m_N;
    double u = m_strategy->u(sig, b, dt);
    double d = m_strategy->d(sig, b, dt);
    double p = m_strategy->p(sig, b, dt);
    double discount = std::exp(-r * dt);
    
    // create struct for output
    
    TreeValues output;
    
    // create and fill vector
    std::vector<double> nodes;
    nodes.reserve(m_N + 1);
    
    for (long index = 0; index < m_N + 1; ++index)
    {
        double node_spot = S * std::pow(u,index) * std::pow(d,m_N-index);
        nodes.push_back(payoff(node_spot, K, parameter));
    }
    
    // roll back the vector
    
    double u_over_d = u / d;
    
    for (long column_index = m_N - 1; column_index >= 0; --column_index)
    {
        
        // find price at lowest node
        double lowest_node_price = S * std::pow(d, column_index);
        
        for (long vector_index = 0; vector_index <= column_index; ++vector_index)
        {
            double value = discount * (p * nodes[vector_index + 1] + (1 - p) * nodes[vector_index]);
            
            // check for early exercise
            if constexpr (OptionType::allow_early_exercise)
            {
                value = std::max(value, payoff(lowest_node_price, K, parameter));
            }
            
            nodes[vector_index] = value;
            
            // move up the lowest node price
            lowest_node_price *= u_over_d;
        }
        // calculate gamma when 3 columns left
        if (column_index == 2)
        {
            double upper_delta = (nodes[2] - nodes[1])/(S * u * u - S * u * d);
            double lower_delta = (nodes[1] - nodes[0])/(S * u * d - S * d * d);
            output.gamma = (upper_delta - lower_delta)/ (0.5 * (S * u * u - S * d * d));
        }
        // calculate delta when 2 columns left
        if (column_index == 1)
        {
            output.delta = (nodes[1] - nodes[0]) / (S * u - S * d);
        }
    }
    
    output.price = nodes[0];
    
    return output;
}

template <typename OptionType>
double BinomialEngine<OptionType>::Price(OptionTypeParameter parameter) const
{
    return Tree(parameter).price;
}

template <typename OptionType>
double BinomialEngine<OptionType>::Delta(OptionTypeParameter parameter) const
{
    return Tree(parameter).delta;
}

template <typename OptionType>
double BinomialEngine<OptionType>::Gamma(OptionTypeParameter parameter) const
{
    return Tree(parameter).gamma;
}

}// namespace option_pricing_lib

#endif // !BinomialEngine_cpp
