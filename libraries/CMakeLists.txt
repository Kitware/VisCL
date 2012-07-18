
set(OCL_SRC_FILES
    gaussian_smooth.cl
    hessian.cl
    track_descr_match.cl
    BRIEF.cl
)

set(CXX_SRC_FILES
    cl_manager.h               cl_manager.cxx
    cl_task.h                  cl_task.cxx
    cl_task_registry.h         cl_task_registry.cxx
    hessian.h                  hessian.cxx
    gaussian_smooth.h          gaussian_smooth.cxx
    track_descr_match.h        track_descr_match.cxx
    cl_image_3d.h              cl_image_3d.cxx
    BRIEF.h                    BRIEF.cxx
    cl_image.h                 cl_image.cxx
    cl_buffer.h                cl_buffer.cxx
    cl_header.h                descriptor.h
)

encode_opencl_sources(cl_strings.cxx ${OCL_SRC_FILES})


add_library(viscl ${OCL_SRC_FILES} ${CXX_SRC_FILES} cl_strings.cxx)
target_link_libraries(viscl ${OPENCL_LIBRARIES})

install( TARGETS viscl EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl
  APPEND FILE ${viscl_export_file}
)


if (VXL_FOUND)
  add_subdirectory(vxl)
endif ()


if(vistk_FOUND AND vidtk_FOUND)
  add_subdirectory(processes)
endif()


