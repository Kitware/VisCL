
cmake_minimum_required(VERSION 2.8)

# This is a CMake helper script used to stringify OpenCL source code
# The reason for needing this script is to update the stringified files at
# build time instead of at configure time.
# The solution is to call
#
# add_custom_command(OUTPUT file.cxx
#   COMMAND ${CMAKE_COMMAND}
#      -D SRC_PATH:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}
#      -D CL_FILES:STRING=${OpenCL_files}
#      -D CXX_FILE:FILEPATH=file.cxx
#      -P ${OPENCL_STRINGIFY}
#    DEPENDS ${ARGN}
#  )


separate_arguments(CL_FILES)

file(WRITE "${CXX_FILE}" "")
foreach(CL_FILE ${CL_FILES})
  set(CL_FILE "${SRC_PATH}/${CL_FILE}")
  file(READ "${CL_FILE}" CL_CODE)
  get_filename_component(CL_NAME "${CL_FILE}" NAME_WE)
  string(REGEX REPLACE "\\\\"  "\\\\\\\\"  code "${CL_CODE}")
  string(REGEX REPLACE "\n"  "\\\\\n" code "${code}")
  string(REGEX REPLACE "\n"  "\\n\\\\n"    code "${code}")
  string(REGEX REPLACE "\""  "\\\\\"" code "${code}")
  file(APPEND "${CXX_FILE}" "const char *${CL_NAME}_source = \"${code}\";\n\n\n")
endforeach()


