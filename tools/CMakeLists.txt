add_executable(track track.cxx )
target_link_libraries(track ${OPENCL_LIBRARIES} vil vil_algo vul viscl)

add_executable(smooth smooth.cxx)
target_link_libraries(smooth ${OPENCL_LIBRARIES} vil vil_algo vul viscl ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY}) 
