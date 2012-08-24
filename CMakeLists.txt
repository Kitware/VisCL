cmake_minimum_required(VERSION 2.8)
project(viscl)

set(viscl_cmake_dir "${viscl_SOURCE_DIR}/CMake")
set(CMAKE_MODULE_PATH ${viscl_cmake_dir} ${CMAKE_MODULE_PATH})

set(viscl_export_file
  "${viscl_BINARY_DIR}/viscl-config-targets.cmake")
# clear the export file before we start appending to it
file(REMOVE ${viscl_export_file})

find_package(OpenCL REQUIRED)

include(viscl-macros)

set(viscl_boost_version 1.47)
set(BOOST_ROOT "" CACHE PATH "The root path to Boost")
option(Boost_USE_STATIC_LIBS "Use a statically-linked Boost" OFF)
find_package(Boost ${viscl_boost_version} REQUIRED COMPONENTS chrono system)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if (WIN32)
  add_definitions(-DBOOST_ALL_NO_LIB)
endif (WIN32)

option(BUILD_VXL_SUPPORT "Build support for interfacing with VXL")
if (BUILD_VXL_SUPPORT)
  find_package(VXL REQUIRED)

  # include the standard stuff, such as Dart test
  set(VXL_PROVIDE_STANDARD_OPTIONS 1)
  include(${VXL_CMAKE_DIR}/UseVXL.cmake)

  include_directories(SYSTEM ${VXL_BRL_INCLUDE_DIR})
  include_directories(SYSTEM ${VXL_RPL_INCLUDE_DIR})
  include_directories(SYSTEM ${VXL_GEL_INCLUDE_DIR}/mrc)
endif ()


include_directories(SYSTEM ${OPENCL_INCLUDE_DIR})
include_directories(libraries)

add_subdirectory(libraries)
add_subdirectory(tools)

option(VISCL_ENABLE_TESTING "Build tests" OFF)
if (VISCL_ENABLE_TESTING)
  add_subdirectory(tests)
endif (VISCL_ENABLE_TESTING)


# Construct the include directories.
SET(viscl_INCLUDE_DIR_CONFIG ${viscl_SOURCE_DIR}/libraries)


CONFIGURE_FILE(${viscl_SOURCE_DIR}/viscl-config.cmake.in
               ${viscl_BINARY_DIR}/viscl-config.cmake @ONLY IMMEDIATE)

if (WIN32)
  set(cmakedir cmake)
else (WIN32)
  set(cmakedir lib${LIB_SUFFIX}/cmake/viscl)
endif (WIN32)


configure_file("${viscl_cmake_dir}/viscl-macros.cmake"
               "${viscl_BINARY_DIR}/viscl-macros.cmake"
               COPYONLY)
configure_file("${viscl_cmake_dir}/viscl-stringify.cmake"
               "${viscl_BINARY_DIR}/viscl-stringify.cmake"
               COPYONLY)

install(
  FILES       "${viscl_BINARY_DIR}/viscl-macros.cmake"
  DESTINATION "${cmakedir}"
  COMPONENT   development)
install(
  FILES       "${viscl_BINARY_DIR}/viscl-stringify.cmake"
  DESTINATION "${cmakedir}"
  COMPONENT   development)
install(
  EXPORT      viscl_exports
  DESTINATION "${cmakedir}"
  FILE        viscl-config-targets.cmake
  COMPONENT   development)
