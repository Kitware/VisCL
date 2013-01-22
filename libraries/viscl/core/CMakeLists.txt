
set(SRC_FILES
    buffer.h                buffer.cxx
    header.h
    matrix.h
    image.h                 image.cxx
    image_3d.h              image_3d.cxx
    manager.h               manager.cxx
    task.h                  task.cxx
    program_registry.h      program_registry.cxx
    utils.h                 utils.cxx
)

set(VISCL_DEFAULT_PLATFORM "-1"
  CACHE STRING "The default platform ID to use (-1 is 'first GPU')")

set_source_files_properties(manager.cxx
  PROPERTIES
    COMPILE_DEFINITIONS "DEFAULT_PLATFORM=${VISCL_DEFAULT_PLATFORM}")

add_library(viscl_core ${SRC_FILES})
target_link_libraries(viscl_core ${OPENCL_LIBRARIES})

install( TARGETS viscl_core EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_core
  APPEND FILE ${viscl_export_file}
)

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/viscl
  COMPONENT development
  FILES_MATCHING PATTERN "*.h"
)
