//
//  MatrixPricer.hpp
//  option_pricing_lib
//
//  header file for a MatrixPricer class template for one-factor plain options
//  an instance of MatrixPricer is a N * number of parameters matrix of appropriate Option parameteres

#ifndef MatrixPricer_hpp
#define MatrixPricer_hpp

#include "Option.hpp"
#include "GridUtilities.hpp"

#include <vector>
#include <iostream>
#include <functional>
#include <string>

namespace option_pricing_lib
{
template <typename OptionType>
class MatrixPricer
{
private:
    
    std::vector<OptionParameter> m_columns; // column labels
    std::vector<std::vector<double>> m_data; // matrix data
    
public:
    
    // constructors, destructor
    
    MatrixPricer(); // default constructor
    MatrixPricer(const MatrixPricer<OptionType>& reference); // copy constructor
    MatrixPricer(const std::vector<OptionParameter>& column_labels, const std::vector<std::vector<double>>& reference); // custom constructor with m_data set to reference
    
    virtual ~MatrixPricer(); // destructor
    
    // static matrix creators
    
    static MatrixPricer<OptionType> Grid(const OptionType& base_option, OptionParameter parameter_1, const std::vector<double>& p1_values, OptionParameter parameter_2, const std::vector<double>& p2_values);
    static MatrixPricer<OptionType> ReadCSV(const std::string& file_path);
    
    // selectors
    
    void Display(std::ostream& os) const; // output instance visualization to given stream
    void WriteCSV(const std::string& file_path) const; // output instance to csv
    
    const std::vector<std::vector<double>>& Data() const; // return const reference to m_data for reading purposes
    
    const std::vector<OptionParameter>& Columns() const; // return const reference to m_columns for reading purposes
    
    // modifiers
    
    void Data(const std::vector<OptionParameter>& column_labels, const std::vector<std::vector<double>>& reference); // set m_data to reference
    
    // operator overloading
    
    MatrixPricer<OptionType>& operator = (const MatrixPricer<OptionType>& reference); // assignment operator
};

// matrix functions

template <typename OptionType>
std::vector<double> MatrixHelper(const MatrixPricer<OptionType>& reference, const std::function<double(const Option&)>& input_func); // helper function

template <typename OptionType>
std::vector<double> MatrixPrice(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter); // call/put matrix pricer function

template <typename OptionType>
std::vector<double> MatrixDelta(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter); // call/put delta matrix function

template <typename OptionType>
std::vector<double> MatrixGamma(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter); // call/put gamma matrix function

template <typename OptionType>
std::vector<double> MatrixVega(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter); // call/put vega matrix function

template <typename OptionType>
std::vector<double> MatrixTheta(const MatrixPricer<OptionType>& reference, OptionTypeParameter option_type_parameter); // call/put theta matrix function

template <typename OptionType>
std::vector<std::vector<double>> MatrixTable(const OptionType& base_option, OptionParameter parameter_1, const std::vector<double>& p1_values, OptionParameter parameter_2, const std::vector<double>& p2_values, const std::function<std::vector<double>(const MatrixPricer<OptionType>&, OptionTypeParameter)>& input_func, OptionTypeParameter option_type);

}// namespace option_pricing_lib

#ifndef MatrixPricer_cpp
#include "MatrixPricer.cpp"
#endif

#endif /* MatrixPricer_hpp */
