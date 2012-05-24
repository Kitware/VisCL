
set(OCL_SRC_FILES
    algo/gaussian_smooth.cl
    algo/FAST.cl
)

set(CXX_SRC_FILES
    cl_manager.h               cl_manager.cxx
    cl_task.h                  cl_task.cxx
    cl_task_registry.h         cl_task_registry.cxx
    algo/FAST.h                algo/FAST.cxx
    algo/gaussian_smooth.h     algo/gaussian_smooth.cxx
)

encode_opencl_sources(cl_strings.cxx ${OCL_SRC_FILES})


add_library(libcl ${OCL_SRC_FILES} ${CXX_SRC_FILES} cl_strings.cxx)

target_link_libraries(libcl ${OPENCL_LIBRARIES} vil vnl)
