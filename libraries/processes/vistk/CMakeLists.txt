project(vistk_processes_porting)

##################################################
# Processes that are manually wrapped up in the
# new pipeline from the old pipeline should be
# added to this library.
##################################################

include_directories(${CMAKE_SOURCE_DIR}/libraries)

# Add the source files to this variable.
set(processes_porting_srcs
  common/vistk_process.h
  common/vistk_process.cxx
  common/vistk_process_macros.h
  conversion/timestamp.h
  conversion/timestamp.cxx
  registration.h
  registration.cxx
  vistk_track_descr_match_process.h
  vistk_track_descr_match_process.cxx
  )

add_vistk_plugin(viscl2vistk_processes
  MAKE_VISCL_PROCESSES_PORTING_LIB
  ${processes_porting_srcs})
# Add any libraries that are necessary here.
target_link_libraries(viscl2vistk_processes
  viscl_vidtk_processes
  vistk_pipeline
  vistk_utilities)
