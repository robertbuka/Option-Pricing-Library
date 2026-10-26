//
//  MatrixPricer.cpp
//  option_pricing_lib
//
//  implementation of MatrixPricer class template functionality
//  and compatible global functions

#ifndef MatrixPricer_cpp
#define MatrixPricer_cpp

#include "MatrixPricer.hpp"
#include "EuropeanOption.hpp"
#include <boost/algorithm/string/trim.hpp>

#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <set>
#include <fstream>
#include <sstream>

namespace option_pricing_lib
{

template <typename OptionType>
MatrixPricer<OptionType>::MatrixPricer(): m_columns(OptionType::label_parameters), m_data() {} // default constructor

template <typename OptionType>
MatrixPricer<OptionType>::MatrixPricer(const MatrixPricer& reference): m_columns(reference.m_columns), m_data(reference.m_data) {} // copy constructor

template <typename OptionType>
MatrixPricer<OptionType>::MatrixPricer(const std::vector<OptionParameter>& column_labels, const std::vector<std::vector<double>>& reference)
{// custom constructor with m_data set to reference, m_columns to column_labels
    
    // check for invalid column_labels
    std::set<OptionParameter> input_labels(column_labels.begin(), column_labels.end());
    std::set<OptionParameter> option_labels(OptionType::label_parameters.begin(), OptionType::label_parameters.end());
    if (input_labels != option_labels || OptionType::label_parameters.size() != column_labels.size()) throw std::invalid_argument("MatrixPricer<OptionType>| Matrix constructor: invalid column labels");
    
    // check for mismatching inner vectors
    for (const auto& inner_vector: reference)
    {
        if (inner_vector.size() != column_labels.size()) throw std::invalid_argument("MatrixPricer<OptionType>| Matrix constructor: inner vectors' size does not match column labels");
    }
    
    m_columns = column_labels;
    m_data = reference;
}

template <typename OptionType>
MatrixPricer<OptionType>::~MatrixPricer() {} // destructor

template <typename OptionType>
void MatrixPricer<OptionType>::Display(std::ostream& os) const
{// output instance visualization to given stream
    
    bool first_value = true;
    
    for (const auto& label: m_columns)
    {
        if (!first_value)
        {
            os << ", " << label;
        } else {
            os << label;
        }
        first_value = false;
    }
    os << "\n";
    
    // output all data
    
    for (const auto& inner_vector: m_data)
    {
        // each row starts with its first value
        first_value = true;
        for (const auto& value: inner_vector)
        {
            if (!first_value)
            {
                os << ", " << value;
            } else {
                os << value;
            }
            first_value = false;
        }
        os << "\n";
    }
}

template <typename OptionType>
void MatrixPricer<OptionType>::WriteCSV(const std::string& file_path) const
{// output instance to csv
    std::ofstream csv_file(file_path);
    
    // check for valid path
    if (!csv_file.is_open()) throw std::invalid_argument("MatrixPricer| invalid file path: " + file_path);
    
    // set double precision
    csv_file << std::setprecision(17);
    
    Display(csv_file);
    csv_file.close();
}

template <typename OptionType>
MatrixPricer<OptionType> MatrixPricer<OptionType>::ReadCSV(const std::string& file_path)
{
    std::ifstream csv_file(file_path);
    
    // check for valid path
    if (!csv_file.is_open()) throw std::invalid_argument("MatrixPricer| ReadCSV: invalid file path: " + file_path);
    
    // get all lines from csv
    std::vector<std::string> all_lines_vector;
    for (std::string line; std::getline(csv_file, line);)
    {
        all_lines_vector.push_back(line);
    }
    
    // check for empty file
    if (all_lines_vector.empty()) throw std::invalid_argument("MatrixPricer| ReadCSV: input file is empty");
    
    // extract header
    std::stringstream first_line;
    first_line << all_lines_vector[0];
    std::vector<std::string> header;
    
    for (std::string parameter; std::getline(first_line, parameter, ',');)
    {
        header.push_back(parameter);
    }
    
    // convert header vector into vector of OptionParameter
    std::vector<OptionParameter> column_labels;
    
    for (const auto& string_parameter: header)
    {
        // using string stream convert strings into OptionParameter
        OptionParameter parameter;
        
        std::stringstream ss;
        ss << string_parameter;
        ss >> parameter;
        
        column_labels.push_back(parameter);
    }
    
    // extract numerical data
    std::vector<std::vector<double>> data;
    
    for (auto it = all_lines_vector.begin() + 1; it != all_lines_vector.end(); ++it)
    {
        // check for empty line
        boost::algorithm::trim(*it);
        if (*it == "") continue;
        
        std::vector<double> row;
        std::stringstream line;
        line << *it;
        
        // convert strings to doubles
        int column_counter = 1;
        for (std::string value; std::getline(line, value, ',');)
        {
            try
            {
                std::size_t elements_used;
                boost::algorithm::trim(value);
                double d_value = std::stod(value, &elements_used);
                
                if (elements_used != value.length()) throw std::exception();
                
                row.push_back(d_value);
            }
            catch (const std::exception&)
            {
                throw std::invalid_argument("MatrixPricer| ReadCSV: invalid value row " + std::to_string(it - all_lines_vector.begin() + 1) + ", column " + std::to_string(column_counter) + " '" + value + "'");
            }
            
            column_counter++;
        }
        data.push_back(row);
    }
    
    csv_file.close();
    
    return MatrixPricer<OptionType>(column_labels,data);
}

template <typename OptionType>
const std::vector<std::vector<double>>& MatrixPricer<OptionType>::Data() const
{// return const reference to m_data for reading purposes
    return m_data;
}

template <typename OptionType>
const std::vector<OptionParameter>& MatrixPricer<OptionType>::Columns() const
{// return const reference to m_columns for reading purposes
    return m_columns;
}

template <typename OptionType>
void MatrixPricer<OptionType>::Data(const std::vector<OptionParameter>& column_labels, const std::vector<std::vector<double>>& reference)
{// set m_data to reference
    
    // check for invalid column_labels
    std::set<OptionParameter> input_labels(column_labels.begin(), column_labels.end());
    std::set<OptionParameter> option_labels(OptionType::label_parameters.begin(), OptionType::label_parameters.end());
    if (input_labels != option_labels || OptionType::label_parameters.size() != column_labels.size()) throw std::invalid_argument("MatrixPricer<OptionType>| Matrix constructor: invalid column labels");
    
    // check for mismatching inner vectors
    for (const auto& inner_vector: reference)
    {
        if (inner_vector.size() != column_labels.size()) throw std::invalid_argument("MatrixPricer<OptionType>| Matrix constructor: inner vectors' size does not match column labels");
    }
    
    m_columns = column_labels;
    m_data = reference;
}

template <typename OptionType>
MatrixPricer<OptionType>& MatrixPricer<OptionType>::operator = (const MatrixPricer& reference)
{// assignment operator
    if (&reference == this) return *this; // check for self assignment
    
    m_columns = reference.m_columns;
    m_data = reference.m_data;
    
    return *this;
}

template <typename OptionType>
std::vector<double> MatrixHelper(const MatrixPricer<OptionType>& reference, const std::function<double(const Option&)>& input_func)
{
    OptionType internal_option;
    std::vector<double> output;
    output.reserve(reference.Data().size());
    
    for (std::size_t row_index = 0; row_index < reference.Data().size(); ++row_index)
    {
        for (std::size_t column_index = 0; column_index < reference.Columns().size(); ++column_index)
        {
            internal_option.SetParameter(reference.Columns()[column_index], reference.Data()[row_index][column_index]);
        }
        output.push_back(input_func(internal_option));
    }
    
    return output;
}

template <typename OptionType>
std::vector<double> MatrixPrice(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter)
{// return a vector containing call/put prices
    switch (option_type_parameter)
    {
        case OptionTypeParameter::Call:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.CallPrice(); });
            break;
        case OptionTypeParameter::Put:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.PutPrice(); });
            break;
        default:
            throw std::invalid_argument("MatrixPrice| invalid option type parameter");
            break;
    }
}

template <typename OptionType>
std::vector<double> MatrixDelta(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter)
{// return a vector containing call/put deltas
    switch (option_type_parameter)
    {
        case OptionTypeParameter::Call:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.CallDelta(); });
            break;
        case OptionTypeParameter::Put:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.PutDelta(); });
            break;
        default:
            throw std::invalid_argument("MatrixDelta| invalid option type parameter");
            break;
    }
}

template <typename OptionType>
std::vector<double> MatrixGamma(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter)
{// return a vector containing call/put gammas
    switch (option_type_parameter)
    {
        case OptionTypeParameter::Call:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.CallGamma(); });
            break;
        case OptionTypeParameter::Put:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.PutGamma(); });
            break;
        default:
            throw std::invalid_argument("MatrixGamma| invalid option type parameter");
            break;
    }
}

template <typename OptionType>
std::vector<double> MatrixVega(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter)
{// return a vector containing call/put vegas
    switch (option_type_parameter)
    {
        case OptionTypeParameter::Call:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.CallVega(); });
            break;
        case OptionTypeParameter::Put:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.PutVega(); });
            break;
        default:
            throw std::invalid_argument("MatrixVega| invalid option type parameter");
            break;
    }
}

template <typename OptionType>
std::vector<double> MatrixTheta(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter)
{// return a vector containing call/put thetas
    switch (option_type_parameter)
    {
        case OptionTypeParameter::Call:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.CallTheta(); });
            break;
        case OptionTypeParameter::Put:
            return MatrixHelper(reference, [](const Option& internal_option){ return internal_option.PutTheta(); });
            break;
        default:
            throw std::invalid_argument("MatrixTheta| invalid option type parameter");
            break;
    }
}

template <typename OptionType>
MatrixPricer<OptionType> MatrixPricer<OptionType>::Grid(const OptionType& base_option, OptionParameter parameter_1, const std::vector<double>& p1_values, OptionParameter parameter_2, const std::vector<double>& p2_values)
{
    // check for invalid input
    auto count_1 = std::count(OptionType::label_parameters.begin(),OptionType::label_parameters.end(),parameter_1);
    auto count_2 = std::count(OptionType::label_parameters.begin(),OptionType::label_parameters.end(),parameter_2);
    if (count_1 == 0 || count_2 == 0) throw std::invalid_argument("MatrixPricer| Grid: invalid option parameter");
    if (parameter_1 == parameter_2) throw std::invalid_argument("MatrixPricer| Grid: input parameters must be different");
    if (p1_values.empty() || p2_values.empty()) throw std::invalid_argument("MatrixPricer| Grid: input vectors must not be empty");
    
    // create data for output and reserve capacity
    std::vector<std::vector<double>> data;
    data.reserve(p1_values.size() * p2_values.size());
    
    // create a flattened matrix
    for (const auto& val_1: p1_values)
    {
        for (const auto& val_2: p2_values)
        {
            std::vector<double> row;
            for (const auto& label: OptionType::label_parameters)
            {
                if (label == parameter_1) {
                    row.push_back(val_1);
                } else if (label == parameter_2) {
                    row.push_back(val_2);
                } else {
                    row.push_back(base_option.GetParameter(label));
                }
            }
            data.push_back(row);
        }
    }
        
    return MatrixPricer<OptionType>(OptionType::label_parameters,data);
}

template <typename OptionType>
std::vector<std::vector<double>> MatrixTable(const OptionType& base_option, OptionParameter parameter_1, const std::vector<double>& p1_values, OptionParameter parameter_2, const std::vector<double>& p2_values, const std::function<std::vector<double>(const MatrixPricer<OptionType>&, OptionTypeParameter)>& input_func, OptionTypeParameter option_type)
{
    MatrixPricer<OptionType> internal_matrix = MatrixPricer<OptionType>::Grid(base_option, parameter_1, p1_values, parameter_2, p2_values);
    return Reshaper(input_func(internal_matrix, option_type), p1_values.size(), p2_values.size());
}

}// namespace option_pricing_lib

#endif // !MatrixPricer_cpp
