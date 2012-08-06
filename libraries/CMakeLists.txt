

add_subdirectory(viscl)


if (VXL_FOUND)
  add_subdirectory(vxl)
endif ()


if(vistk_FOUND AND vidtk_FOUND)
  add_subdirectory(processes)
endif()
