cmake_minimum_required(VERSION 2.8)
project(viscl)

set(CMAKE_MODULE_PATH "${viscl_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(VXL)
find_package(OpenCL REQUIRED)

set(viscl_boost_version 1.47)
set(BOOST_ROOT "" CACHE PATH "The root path to Boost")
option(Boost_USE_STATIC_LIBS "Use a statically-linked Boost" OFF)
find_package(Boost ${viscl_boost_version} REQUIRED COMPONENTS chrono system)
find_package(vidtk QUIET)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if (WIN32)
  add_definitions(-DBOOST_ALL_NO_LIB)
endif (WIN32)

find_package(vistk QUIET)
if (vistk_FOUND)
  include(${vistk_DIR}/vistk-macros.cmake)
  include_directories(SYSTEM ${VISTK_INCLUDE_DIR})
  link_directories(${VISTK_LIBRARY_DIR})
endif (vistk_FOUND)

# include the standard stuff, such as Dart test
set(VXL_PROVIDE_STANDARD_OPTIONS 1)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

include_directories(SYSTEM ${VXL_BRL_INCLUDE_DIR})
include_directories(SYSTEM ${VXL_RPL_INCLUDE_DIR})
include_directories(SYSTEM ${VXL_GEL_INCLUDE_DIR}/mrc)
include_directories(SYSTEM ${VIDTK_INCLUDE_DIRS})

include_directories(SYSTEM ${OPENCL_INCLUDE_DIR})

include_directories(libraries)

add_subdirectory(libraries)
add_subdirectory(tools)

# Save library dependencies.
SET(viscl_LIBRARY_DEPENDS_FILE ${viscl_BINARY_DIR}/visclLibraryDepends.cmake)
EXPORT_LIBRARY_DEPENDENCIES(${viscl_LIBRARY_DEPENDS_FILE})

# Construct the include directories.
SET(viscl_INCLUDE_DIR_CONFIG ${viscl_SOURCE_DIR})


CONFIGURE_FILE(${viscl_SOURCE_DIR}/visclConfig.cmake.in
               ${viscl_BINARY_DIR}/visclConfig.cmake @ONLY IMMEDIATE)


#----------------------------------
