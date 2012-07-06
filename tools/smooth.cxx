/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vcl_iostream.h>

#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

#include <vil/algo/vil_gauss_filter.h>

#include "cl_manager.h"
#include "cl_task_registry.h"
#include "gaussian_smooth.h"

#include <boost/chrono.hpp>

int main(int argc, char *argv[])
{
  cl_manager::inst()->report_system_specs();

  vil_image_view<vxl_byte> img_color = vil_load(argv[1]);
  vil_image_view<vxl_byte> img, output;
  vil_convert_planes_to_grey(img_color, img);

  int radii[3] = {2, 3, 4};

  gaussian_smooth_t smoother = NEW_VISCL_TASK(gaussian_smooth);
  cl_image img_cl = cl_manager::inst()->create_image<vxl_byte>(img);

  const int iter = 100;
  for (int r = 0; r < 3; r++)
  {
    boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();
    for (int i = 0; i < iter; i++)
      cl_image result = smoother->smooth( img_cl, 2.0, radii[r]);
    boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - start;
    vcl_cout << "viscl took " << sec.count() << " seconds to smooth a " << img.ni() << "x" << img.nj() << " img " << iter << "x w/ kernel size=" << 2*radii[r]+1 << ".\n";
    start = boost::chrono::system_clock::now();
    for (int i = 0; i < iter; i++)
      vil_gauss_filter_2d<vxl_byte, vxl_byte>(img, output, 2.0, radii[r]);
    sec = boost::chrono::system_clock::now() - start;
    vcl_cout << "VXL took "   << sec.count() << " seconds to smooth a " << img.ni() << "x" << img.nj() << " img " << iter << "x w/ kernel size=" << 2*radii[r]+1 << ".\n";
  }

  return 0;
}
