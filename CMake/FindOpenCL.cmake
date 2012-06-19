# - Try to find OpenCL
# Once done this will define
#  
#  OPENCL_FOUND         - system has OpenCL
#  OPENCL_INCLUDE_DIR  - the OpenCL include directory
#  OPENCL_LIBRARIES     - link these to use OpenCL
#
# WIN32 should work, but is untested

IF (WIN32)

        FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h )

        # TODO this is only a hack assuming the 64 bit library will
        # not be found on 32 bit system
        FIND_LIBRARY(OPENCL_LIBRARIES OpenCL )

ELSE (WIN32)

        # Unix style platforms
        # We also search for OpenCL in the NVIDIA SDK default location
        FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h ~/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc/ )
        FIND_LIBRARY(OPENCL_LIBRARIES OpenCL 
          ENV LD_LIBRARY_PATH
        )

ENDIF (WIN32)

SET( OPENCL_FOUND "NO" )
IF(OPENCL_LIBRARIES )
        SET( OPENCL_FOUND "YES" )
ENDIF(OPENCL_LIBRARIES)

MARK_AS_ADVANCED(
  OPENCL_INCLUDE_DIR
)


set(OPENCL_STRINGIFY ${CMAKE_CURRENT_LIST_DIR}/stringify.cmake)
if (NOT EXISTS ${OPENCL_STRINGIFY})
  message(FATAL_ERROR "stringify.cmake not found in ${CMAKE_CURRENT_LIST_DIR}")
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
