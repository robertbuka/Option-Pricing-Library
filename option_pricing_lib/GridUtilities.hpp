//
//  GridUtilities.hpp
//  option_pricing_lib
//
//  header file for Mesher() and Reshaper() functions
//  Mesher(): if the range is not a whole number of mesh_size, last step is adjusted
//  Reshaper(): convert a flattened vector into 2D matrix

#ifndef GridUtilities_hpp
#define GridUtilities_hpp

#include <vector>

namespace option_pricing_lib
{
std::vector<double> Mesher(double start_value, double end_value, double mesh_size); // mesher function

std::vector<std::vector<double>> Reshaper(const std::vector<double>& flat, std::size_t values_1_size, std::size_t values_2_size); // reshape function
}// namespace option_pricing_lib

#endif // !GridUtilities_hpp

