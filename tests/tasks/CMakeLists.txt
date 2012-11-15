
set(libraries
  viscl_tasks  ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if (BUILD_VXL_SUPPORT)
  list(APPEND libraries viscl_vxl)
  add_definitions( -DHAS_VXL )
endif (BUILD_VXL_SUPPORT)

viscl_build_test(gaussian_smooth libraries test_gaussian_smooth.cxx)
viscl_build_test(warp_image libraries test_warp_image.cxx)
viscl_build_test(jitter_difference libraries test_jitter_difference.cxx)

viscl_make_test(gaussian_smooth smooth)
if (BUILD_VXL_SUPPORT)
  # additional tests using VXL
  viscl_make_test(gaussian_smooth smooth_vxl)
  viscl_make_test(warp_image warp_vxl)
  viscl_make_test(jitter_difference jitter_diff_vxl)
endif (BUILD_VXL_SUPPORT)
