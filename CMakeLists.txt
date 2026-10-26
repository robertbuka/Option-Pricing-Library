cmake_minimum_required(VERSION 3.20)

# set the project name
project(option_pricing_lib LANGUAGES CXX)

# set C++ standard
set(CMAKE_CXX_STANDARD 20)                  
set(CMAKE_CXX_STANDARD_REQUIRED ON)         # fail if the compiler can't do it
set(CMAKE_CXX_EXTENSIONS OFF)

# build in Release by default
if(NOT CMAKE_BUILD_TYPE)                   
    set(CMAKE_BUILD_TYPE Release)           
endif()

# find boost
find_package(Boost REQUIRED)

# add library
add_library(option_pricing_lib
	option_pricing_lib/AmericanOption.cpp
	option_pricing_lib/AmericanPerpetualOption.cpp
	option_pricing_lib/BinomialStrategy.cpp
	option_pricing_lib/EuropeanOption.cpp
	option_pricing_lib/GridUtilities.cpp
	option_pricing_lib/Option.cpp
)

# target include directories
target_include_directories(option_pricing_lib PUBLIC option_pricing_lib)
target_link_libraries(option_pricing_lib PUBLIC Boost::headers)
target_compile_options(option_pricing_lib PRIVATE -Wall -Wextra -Wpedantic) # warnings for its own files

# add executables

add_executable(validation validation/validation.cpp)
target_link_libraries(validation PRIVATE option_pricing_lib)
target_compile_options(validation PRIVATE -Wextra -Wall -Wpedantic)

add_executable(test option_pricing_lib/test.cpp)
target_link_libraries(test PRIVATE option_pricing_lib)
target_compile_options(test PRIVATE -Wextra -Wall -Wpedantic)

