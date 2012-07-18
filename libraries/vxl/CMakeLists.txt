

set(CXX_SRC_FILES
    vxl_transfer.h             vxl_transfer.cxx
    vxl_tasks.h                vxl_tasks.cxx
    vxl_cl_util.h              vxl_cl_util.cxx
)


add_library(viscl_vxl ${CXX_SRC_FILES})
target_link_libraries(viscl_vxl viscl vil vil_algo vnl)


install( TARGETS viscl_vxl EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_vxl
  APPEND FILE ${viscl_export_file}
)
