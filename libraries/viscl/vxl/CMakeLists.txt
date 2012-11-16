

set(SRC_FILES
    transfer.h                 transfer.cxx
    tasks.h                    tasks.cxx
    util.h                     util.cxx
    conversion.h               conversion.cxx
)


add_library(viscl_vxl ${SRC_FILES})
target_link_libraries(viscl_vxl viscl_tasks vil vil_algo vnl vgl_algo vgl)


install( TARGETS viscl_vxl EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_vxl
  APPEND FILE ${viscl_export_file}
)

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/viscl
  COMPONENT development
  FILES_MATCHING PATTERN "*.h"
)
