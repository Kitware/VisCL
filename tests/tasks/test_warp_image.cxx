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
#include <vil/vil_nearest_interp.h>
#endif

float frac(float x)
{
  return x - floor(x);
}

#ifdef HAS_VXL

//OpenCL spec linear interpolator
vxl_byte linear_interp(const vil_image_view<vxl_byte> &src, float u, float v)
{
  int i0 = (int)floor(u - 0.5f);
  int j0 = (int)floor(v - 0.5f);
  int i1 = (int)(floor(u - 0.5f) + 1.0f);
  int j1 = (int)(floor(v - 0.5f) + 1.0f);

  float a = frac(u - 0.5f);
  float b = frac(v - 0.5f);
  float t00 = 0.0, t10 = 0.0, t01 = 0.0, t11 = 0.0;
  if (i0 >= 0 && i0 < src.ni())
  {
    if (j0 >= 0 && j0 < src.nj())
      t00 = static_cast<float>(src(i0,j0));
    if (j1 >= 0 && j1 < src.nj())
      t01 =  static_cast<float>(src(i0,j1));
  }
  if (i1 >= 0 && i1 < src.ni())
  {
    if (j0 >= 0 && j0 < src.nj())
      t10 = static_cast<float>(src(i1,j0));
    if (j1 >= 0 && j1 < src.nj())
      t11 = static_cast<float>(src(i1,j1));
  }

  return static_cast<vxl_byte>((1.0f - a) * (1.0f - b) * t00
                               + a * (1.0f - b) * t10
                               + (1.0f - a) * b * t01
                               + a * b * t11 );
}

void
warp_image( vil_image_view<vxl_byte> const& src,
            vil_image_view<vxl_byte>& dest,
            vgl_h_matrix_2d<float> const& dest_to_src_homography)
{
  dest.set_size(src.ni(), src.nj(), 1);
  for (unsigned int i = 0; i < dest.ni(); i++)
  {
    for (unsigned int j = 0; j < dest.nj(); j++)
    {
      vgl_homg_point_2d<float> pt(i, j, 1);
      vgl_homg_point_2d<float> warped = dest_to_src_homography(pt);
      dest(i,j) = linear_interp(src, warped.x() / warped.w(), warped.y() / warped.w());
    }
  }
}

#endif

#define EPSILON 1

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
  const unsigned width = 1000;
  const unsigned height = 1000;

  // create a test image with a checkerboard pattern
  vil_image_view<vxl_byte> input_img(width, height), result_img;
  make_checkerboard_image(width, height, input_img.top_left_ptr(), 4);

  vgl_h_matrix_2d<float> H;
  H.set_identity();
  H.set_rotation(1.0f);
  H.set_translation(0.0f, -10.0f);

  // warp the image with the CPU
  vil_image_view<vxl_byte> truth_img(width, height);
  warp_image(input_img, truth_img, H);

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

  //Count differing pixels, for some reason they are not exactly the same (floating pt rounding?)
  unsigned long diff_count = 0;
  for( unsigned j = 0; j < height; ++j )
  {
    for( unsigned i = 0; i < width; ++i )
    {
      if (abs((int)(result_img(i,j) - truth_img(i,j))) > EPSILON )
      {
        ++diff_count;
      }
    }
  }

  float percentage = (100.0f*diff_count)/(width*height);
  if (percentage > 0.1)
  {
    TEST_ERROR("GPU and CPU warping results differ in "
               << percentage << "% of pixels");

    //vil_save(input_img, "test_warp_source.png");
    vil_save(result_img, "test_warp_result.png");
    vil_save(truth_img, "test_warp_truth.png");
  }

#else
  TEST_ERROR("VXL support is not compiled in, this test is invalid");
#endif
}
