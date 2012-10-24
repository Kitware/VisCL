/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <iostream>
#include <cmath>
#include <cstdlib>
#include <exception>

#include <test_common.h>

#include <viscl/core/manager.h>

#include <viscl/tasks/warp_image.h>

#ifdef HAS_VXL
#include <viscl/vxl/transfer.h>
#include <viscl/vxl/tasks.h>
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/vgl_homg_point_2d.h>
#include <vil/vil_save.h>
#include <vil/vil_bilin_interp.h>
#endif

static void run_test(std::string const& test_name);

int
main(int argc, char* argv[])
{
  if (argc != 2)
  {
    TEST_ERROR("Expected one argument");

    return EXIT_FAILURE;
  }

  std::string const test_name = argv[1];

  try
  {
    run_test(test_name);
  }
  catch (std::exception const& e)
  {
    TEST_ERROR("Unexpected exception: " << e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static void test_warp_vxl();

void
run_test(std::string const& test_name)
{
  if (test_name == "warp_vxl")
  {
    test_warp_vxl();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}

void
test_warp_vxl()
{
#ifdef HAS_VXL
  // test parameters
  const unsigned width = 100;
  const unsigned height = 100;

  // create the viscl task
  viscl::warp_image_t warper = NEW_VISCL_TASK(viscl::warp_image);

  // create a test image with a checkerboard pattern
  vil_image_view<vxl_byte> input_img(width, height), result_img;
  make_checkerboard_image(width, height, input_img.top_left_ptr());

  vgl_h_matrix_2d<float> H;
  H.set_identity();
  H.set_rotation(.1f);
  H.set_translation(0.0f, -10.0f);

  vcl_cout << H << "\n";

  // warp the image with the CPU
  vil_image_view<vxl_byte> truth_img(width, height);
  for (unsigned int i = 0; i < truth_img.ni(); i++)
  {
    for (unsigned int j = 0; j < truth_img.nj(); j++)
    {
      vgl_homg_point_2d<float> p(static_cast<float>(i), static_cast<float>(j), 1.0f);
      vgl_homg_point_2d<float> pwarp = H * p;
      double iwarp = pwarp.x() / pwarp.w();
      double jwarp = pwarp.y() / pwarp.w();
      truth_img(i,j) = vil_bilin_interp_safe<vxl_byte>(input_img, iwarp, jwarp);
    }
  }

  // apply the viscl warping task
  try
  {
    viscl::warp_image_vxl(input_img, result_img, H);
  }
  catch (const cl::Error &e)
  {
    TEST_ERROR(e.what() << " (" << e.err() << " : "
               << viscl::print_cl_errstring(e.err()) << ")");
  }

  unsigned long diff_count = 0;
  for( unsigned j = 0; j < height; ++j )
  {
    for( unsigned i = 0; i < width; ++i )
    {
      if (result_img(i,j) != truth_img(i,j))
      {
        ++diff_count;
      }
    }
  }

  if (diff_count > 0)
  {
    TEST_ERROR("GPU and CPU smoothing results differ in "
               << (100.0f*diff_count)/(width*height) << "% of pixels");

    //vil_save(input_img, "test_warp_source.png");
    vil_save(result_img, "test_warp_result.png");
    vil_save(truth_img, "test_warp_truth.png");
  }

#else
  TEST_ERROR("VXL support is not compiled in, this test is invalid");
#endif
}
