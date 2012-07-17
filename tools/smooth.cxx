/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <vcl_iostream.h>
#include <vcl_cstdlib.h>
#include <vcl_ctime.h>
#include <vul/vul_arg.h>

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
  vul_arg<unsigned> arg_iw("-iw", "Image width", 4096);
  vul_arg<unsigned> arg_ih("-ih", "Image height", 4096);
  vul_arg<unsigned> arg_nj("-nj", "# of iterations", 10);

  vul_arg_parse(argc, argv);

  unsigned iw = arg_iw();
  unsigned ih = arg_ih();
  const unsigned iter = arg_nj();

  cl_manager::inst()->report_system_specs();

  vil_image_view<vxl_byte> output, img = vil_image_view<vxl_byte>(iw,ih);
  vcl_srand(vcl_time(NULL));
  for(unsigned j=0; j<img.nj(); ++j)
  {
    for(unsigned i=0; i<img.ni(); ++i)
    {
      img(i,j) = vcl_rand();
    }
  }

  int radii[3] = {2, 3, 4};

  gaussian_smooth_t smoother = NEW_VISCL_TASK(gaussian_smooth);
  cl_image img_cl = cl_manager::inst()->create_image<vxl_byte>(img);


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
