

add_subdirectory(viscl)


if(BUILD_VXL_SUPPORT AND vistk_FOUND AND vidtk_FOUND)
  add_subdirectory(processes)
endif()
