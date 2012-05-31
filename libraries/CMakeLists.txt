
set(OCL_SRC_FILES
    gaussian_smooth.cl
    hessian.cl 
)

set(CXX_SRC_FILES
    cl_manager.h               cl_manager.cxx
    cl_task.h                  cl_task.cxx
    cl_image.h                 cl_image.cxx
    cl_buffer.h                cl_buffer.cxx
    cl_util.h                  cl_util.cxx
    cl_task_registry.h         cl_task_registry.cxx
    hessian.h                  hessian.cxx
    gaussian_smooth.h          gaussian_smooth.cxx
)

encode_opencl_sources(cl_strings.cxx ${OCL_SRC_FILES})


add_library(viscl ${OCL_SRC_FILES} ${CXX_SRC_FILES} cl_strings.cxx)

target_link_libraries(viscl ${OPENCL_LIBRARIES} vil vnl)
