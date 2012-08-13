
set(OPENCL_STRINGIFY ${CMAKE_CURRENT_LIST_DIR}/viscl-stringify.cmake)
if (NOT EXISTS ${OPENCL_STRINGIFY})
  message(FATAL_ERROR "viscl-stringify.cmake not found in ${CMAKE_CURRENT_LIST_DIR}")
endif()


function(encode_opencl_sources cxx_file)
  add_custom_command(OUTPUT ${cxx_file}
    COMMAND ${CMAKE_COMMAND}
      -D SRC_PATH:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}
      -D CL_FILES:STRING="${ARGN}"
      -D CXX_FILE:FILEPATH=${cxx_file}
      -P ${OPENCL_STRINGIFY}
    DEPENDS ${ARGN}
      ${OPENCL_STRINGIFY}
  )
endfunction()
