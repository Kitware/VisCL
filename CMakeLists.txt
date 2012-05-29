cmake_minimum_required(VERSION 2.8)
project(viscl)

set(CMAKE_MODULE_PATH "${viscl_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(VXL)
find_package(OpenCL)
find_package(Boost REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# include the standard stuff, such as Dart test
set(VXL_PROVIDE_STANDARD_OPTIONS 1)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

include_directories(SYSTEM ${VXL_BRL_INCLUDE_DIR})
include_directories(SYSTEM ${VXL_GEL_INCLUDE_DIR}/mrc)
include_directories(SYSTEM ${OPENCL_INCLUDE_DIR})
             
include_directories(libraries)

                      
add_subdirectory(libraries)
add_subdirectory(tools)

