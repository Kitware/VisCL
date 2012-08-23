###################################################################
# Executable tools which are installed
###################################################################

add_executable(viscl_report_opencl_specs report_opencl_specs.cxx)
target_link_libraries(viscl_report_opencl_specs viscl_core)

install(TARGETS viscl_report_opencl_specs DESTINATION bin)


if (BUILD_VXL_SUPPORT)

  add_executable(viscl_time_smooth time_smooth.cxx)
  target_link_libraries(viscl_time_smooth
    viscl_tasks viscl_vxl vil_algo vul  ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

  install(TARGETS viscl_time_smooth DESTINATION bin)


  # TODO: Make this not dependent on VXL.
  # This only depends on VXL for command line parsing.
  add_executable(viscl_time_memory_transfer time_memory_transfer.cxx)
  target_link_libraries(viscl_time_memory_transfer
    vul ${OPENCL_LIBRARIES} ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

  install(TARGETS viscl_time_memory_transfer DESTINATION bin)

endif (BUILD_VXL_SUPPORT)


###################################################################
# Developer testing executables which are not meant for install
###################################################################

if (BUILD_VXL_SUPPORT)

  add_executable(track track.cxx )
  target_link_libraries(track
    viscl_tasks viscl_vxl vul )

endif (BUILD_VXL_SUPPORT)
