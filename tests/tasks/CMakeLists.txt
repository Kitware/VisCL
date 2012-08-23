
set(libraries
  viscl_tasks)

if (BUILD_VXL_SUPPORT)
  list(APPEND libraries viscl_vxl)
  add_definitions( -DHAS_VXL )
endif (BUILD_VXL_SUPPORT)

viscl_build_test(gaussian_smooth libraries test_gaussian_smooth.cxx)

viscl_make_test(gaussian_smooth smooth)
if (BUILD_VXL_SUPPORT)
  # additional tests using VXL
  viscl_make_test(gaussian_smooth smooth_vxl)
endif (BUILD_VXL_SUPPORT)


