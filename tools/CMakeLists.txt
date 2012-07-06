add_executable(track track.cxx )
target_link_libraries(track ${OPENCL_LIBRARIES} vil vil_algo vul viscl)

add_executable(time_smooth time_smooth.cxx)
target_link_libraries(time_smooth ${OPENCL_LIBRARIES} vil vil_algo vul viscl ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

add_executable(time_memory_transfer time_memory_transfer.cxx)
target_link_libraries(time_memory_transfer ${OPENCL_LIBRARIES} vul viscl ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

