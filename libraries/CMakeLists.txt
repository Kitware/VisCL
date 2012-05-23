add_library(libcl 	cl_manager.h cl_manager.cxx
					cl_task.h cl_task.cxx
					cl_task_registry.h cl_task_registry.cxx
					algo/FAST.h algo/FAST.cxx algo/FAST.cl
					algo/gaussian_smooth.h algo/gaussian_smooth.cxx algo/gaussian_smooth.cl
					)
target_link_libraries(libcl ${OPENCL_LIBRARIES} vil vnl)
