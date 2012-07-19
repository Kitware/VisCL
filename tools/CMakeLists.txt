###################################################################
# Executable tools which are installed
###################################################################

add_executable(viscl_report_opencl_specs report_opencl_specs.cxx)
target_link_libraries(viscl_report_opencl_specs viscl)

install(TARGETS viscl_report_opencl_specs DESTINATION bin)


add_executable(viscl_time_smooth time_smooth.cxx)
target_link_libraries(viscl_time_smooth
  viscl vil vil_algo vul  ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

install(TARGETS viscl_time_smooth DESTINATION bin)


add_executable(viscl_time_memory_transfer time_memory_transfer.cxx)
target_link_libraries(viscl_time_memory_transfer
  viscl vul ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

install(TARGETS viscl_time_memory_transfer DESTINATION bin)


###################################################################
# Developer testing executables which are not meant for install
###################################################################

add_executable(track track.cxx )
target_link_libraries(track
  viscl vil vil_algo vul )
