

set(CXX_SRC_FILES
    transfer.h                 transfer.cxx
    tasks.h                    tasks.cxx
    util.h                     util.cxx
)


add_library(viscl_vxl ${CXX_SRC_FILES})
target_link_libraries(viscl_vxl viscl_core vil vil_algo vnl)


install( TARGETS viscl_vxl EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_vxl
  APPEND FILE ${viscl_export_file}
)
