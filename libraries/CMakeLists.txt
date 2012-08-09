

add_subdirectory(viscl)


if(vistk_FOUND AND vidtk_FOUND)
  add_subdirectory(processes)
endif()
