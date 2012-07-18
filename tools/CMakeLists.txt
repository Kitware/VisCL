add_executable(viscl_report_opencl_specs report_opencl_specs.cxx)
target_link_libraries(viscl_report_opencl_specs viscl)

add_executable(track track.cxx )
target_link_libraries(track
  viscl vil vil_algo vul )

add_executable(viscl_time_smooth time_smooth.cxx)
target_link_libraries(viscl_time_smooth
  viscl vil vil_algo vul  ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

add_executable(viscl_time_memory_transfer time_memory_transfer.cxx)
target_link_libraries(viscl_time_memory_transfer
  viscl vul ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

