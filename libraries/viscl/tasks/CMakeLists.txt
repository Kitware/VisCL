
set(OCL_SRC_FILES
    gaussian_smooth.cl
    hessian.cl
    track_descr_match.cl
    BRIEF.cl
    warp_image.cl
    quad_tree.cl
)

set(CXX_SRC_FILES
    BRIEF.h                    BRIEF.cxx
    descriptor.h
    gaussian_smooth.h          gaussian_smooth.cxx
    hessian.h                  hessian.cxx
    track_descr_match.h        track_descr_match.cxx
    warp_image.h               warp_image.cxx
    quad_tree.h                quad_tree.cxx
)

viscl_encode_opencl_sources(cl_strings.cxx ${OCL_SRC_FILES})

function (viscl_kernel_group)
  source_group("Kernels"
    FILES ${ARGN})
endfunction ()

viscl_kernel_group(${OCL_SRC_FILES})

#OCL_SRC_FILES are needed in the add_library so that they show up in Visual Studio
add_library(viscl_tasks ${CXX_SRC_FILES} ${OCL_SRC_FILES} cl_strings.cxx)
target_link_libraries(viscl_tasks viscl_core)

install( TARGETS viscl_tasks EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_tasks
  APPEND FILE ${viscl_export_file}
)

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/viscl
  COMPONENT development
  FILES_MATCHING PATTERN "*.h"
)
