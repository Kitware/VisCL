
set(libraries
  viscl_core)

viscl_build_test(manager libraries test_manager.cxx)

viscl_make_test(manager report_opencl_specs)
viscl_make_test(manager create_image)


