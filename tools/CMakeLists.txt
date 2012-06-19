add_executable(track track.cxx )
target_link_libraries(track ${OPENCL_LIBRARIES} vil vil_algo vul viscl)
