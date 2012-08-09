# This CMake file may be included by projects outside viscl.  It
# configures them to make use of viscl headers and libraries.
#
# The preferred way to use viscl from an outside project with Useviscl.cmake:
#
#  FIND_PACKAGE(viscl)
#  IF(viscl_FOUND)
#    INCLUDE(${viscl_CMAKE_DIR}/Useviscl.cmake)
#  ELSE(viscl_FOUND)
#    MESSAGE("viscl_DIR should be set to the viscl build directory.")
#  ENDIF(viscl_FOUND)

# Load the compiler settings used for viscling.
IF(viscl_BUILD_SETTINGS_FILE)
  INCLUDE(${CMAKE_ROOT}/Modules/CMakeImportBuildSettings.cmake)
  CMAKE_IMPORT_BUILD_SETTINGS(${viscl_BUILD_SETTINGS_FILE})
ENDIF(viscl_BUILD_SETTINGS_FILE)

# Use the standard viscl include directories.
INCLUDE_DIRECTORIES(${viscl_INCLUDE_DIR})

# include the vistk macros
include(${viscl_DIR}/viscl-macros.cmake)

# Use VXL.
IF(NOT viscl_NO_USE_VXL)
  SET(VXL_DIR ${viscl_VXL_DIR})
  FIND_PACKAGE(VXL)
  IF(VXL_FOUND)
    INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)
  ELSE(VXL_FOUND)
    MESSAGE("VXL not found in viscl_VXL_DIR=\"${viscl_VXL_DIR}\".")
  ENDIF(VXL_FOUND)
ENDIF(NOT viscl_NO_USE_VXL)

#find_package(Boost REQUIRED)
#include_directories(${viscl_Boost_INCLUDE_DIR})


