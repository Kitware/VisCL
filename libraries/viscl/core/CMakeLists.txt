
set(CXX_SRC_FILES
    cl_buffer.h                cl_buffer.cxx
    cl_header.h
    cl_image.h                 cl_image.cxx
    cl_image_3d.h              cl_image_3d.cxx
    cl_manager.h               cl_manager.cxx
    cl_task.h                  cl_task.cxx
    cl_task_registry.h         cl_task_registry.cxx
)

add_library(viscl_core ${CXX_SRC_FILES})
target_link_libraries(viscl_core ${OPENCL_LIBRARIES})

install( TARGETS viscl_core EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_core
  APPEND FILE ${viscl_export_file}
)
