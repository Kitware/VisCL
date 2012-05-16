add_executable(klt klt.h klt.cxx )
target_link_libraries(klt ${OPENCL_LIBRARIES} vil vil_algo vul libcl)



