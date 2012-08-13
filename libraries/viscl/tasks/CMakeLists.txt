
set(OCL_SRC_FILES
    gaussian_smooth.cl
    hessian.cl
    track_descr_match.cl
    BRIEF.cl
)

set(CXX_SRC_FILES
    BRIEF.h                    BRIEF.cxx
    descriptor.h
    gaussian_smooth.h          gaussian_smooth.cxx
    hessian.h                  hessian.cxx
    track_descr_match.h        track_descr_match.cxx
)

viscl_encode_opencl_sources(cl_strings.cxx ${OCL_SRC_FILES})


add_library(viscl_tasks ${CXX_SRC_FILES} cl_strings.cxx)
target_link_libraries(viscl_tasks viscl_core)

install( TARGETS viscl_tasks EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_tasks
  APPEND FILE ${viscl_export_file}
)

foreach(fname in ${CXX_SRC_FILES})
  string(REGEX MATCH ".*\\.h" hfile ${fname})
  list(APPEND HEADER_FILES ${hfile})
endforeach()

install( FILES  ${HEADER_FILES}
  DESTINATION "include/viscl/tasks"
  COMPONENT   development)
