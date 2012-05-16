add_library(libcl cl_manager.h cl_manager.cxx)
target_link_libraries(libcl ${OPENCL_LIBRARIES} vil vnl)
