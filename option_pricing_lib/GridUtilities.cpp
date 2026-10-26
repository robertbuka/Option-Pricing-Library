//
//  GridUtilities.cpp
//  option_pricing_lib
//
//  implementation of Mesher() function
//  if the range is not a whole number of mesh_size, last step is adjusted

#include "GridUtilities.hpp"

#include <vector>
#include <cmath>
#include <stdexcept>

namespace option_pricing_lib
{
std::vector<double> Mesher(double start_value, double end_value, double mesh_size)
{// produce monotonically increasing/decreasing array of doubles with increment step equal to mesh_size
    
    // check for valid mesh_size to avoid division by 0
    if (mesh_size == 0) throw std::invalid_argument("Mesher| mesh_size must not be equal to 0");
    
    // check for valid arguments
    if (start_value < end_value && mesh_size <= 0) throw std::invalid_argument("Mesher| for monotonically increasing array, mesh_size must be > 0");
    if (start_value > end_value && mesh_size >= 0) throw std::invalid_argument("Mesher| for monotonically decreasing array, mesh_size must be < 0");
    
    // create output vector
    std::vector<double> output;
    
    //fill output
    output.push_back(start_value);
    
    // check for one-point mesh
    if(start_value == end_value) return output;
    
    for (int counter = 1; counter < std::round(std::abs((end_value - start_value) / mesh_size)); ++counter)
    {
        output.push_back(start_value + counter * mesh_size);
    }
    
    output.push_back(end_value);
    
    return output;
}

std::vector<std::vector<double>> Reshaper(const std::vector<double>& flat, std::size_t values_1_size, std::size_t values_2_size)
{// reshape function
    
    //check for invalid input
    if (flat.size() != values_1_size * values_2_size) throw std::invalid_argument("Reshaper| flat values size does not match tables dimensions");
    
    std::vector<std::vector<double>> output;
    output.reserve(values_1_size);
    
    for (std::size_t outter_counter = 0; outter_counter < values_1_size; ++outter_counter)
    {
        std::vector<double> row;
        row.reserve(values_2_size);
        
        for (std::size_t inner_counter = 0; inner_counter < values_2_size; ++inner_counter)
        {
            row.push_back(flat[outter_counter * values_2_size + inner_counter]);
        }
        output.push_back(row);
    }
    
    return output;
}
}// namespace option_pricing_lib
