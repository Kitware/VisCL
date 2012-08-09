
set(libraries
  viscl_tasks viscl_vxl)

viscl_build_test(gaussian_smooth libraries test_gaussian_smooth.cxx)

viscl_make_test(gaussian_smooth smooth)
viscl_make_test(gaussian_smooth smooth_vxl)


