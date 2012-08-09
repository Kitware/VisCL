
set(SRC_FILES
    buffer.h                buffer.cxx
    header.h
    image.h                 image.cxx
    image_3d.h              image_3d.cxx
    manager.h               manager.cxx
    task.h                  task.cxx
    task_registry.h         task_registry.cxx
)

add_library(viscl_core ${SRC_FILES})
target_link_libraries(viscl_core ${OPENCL_LIBRARIES})

install( TARGETS viscl_core EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_core
  APPEND FILE ${viscl_export_file}
)

foreach(fname in ${SRC_FILES})
  string(REGEX MATCH ".*\\.h" hfile ${fname})
  list(APPEND HEADER_FILES ${hfile})
endforeach()

install( FILES  ${HEADER_FILES}
  DESTINATION "include/viscl/core"
  COMPONENT   development)
