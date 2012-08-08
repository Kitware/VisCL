/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <iostream>
#include <ctime>
#include <vul/vul_arg.h>

#include <vil/vil_image_view.h>

#include <vil/algo/vil_gauss_filter.h>

#include <viscl/core/manager.h>
#include <viscl/core/task_registry.h>
#include <viscl/tasks/gaussian_smooth.h>
#include <vxl/vxl_transfer.h>

#include <boost/chrono.hpp>

int main(int argc, char *argv[])
{
  vul_arg<unsigned> arg_iw("-iw", "Image width", 4096);
  vul_arg<unsigned> arg_ih("-ih", "Image height", 4096);
  vul_arg<unsigned> arg_nj("-nj", "# of iterations", 10);
  vul_arg<bool>     arg_use_cpu("-cpu", "run CPU timing", false);
  vul_arg<bool>     arg_use_gpu("-gpu", "run GPU timing", false);

  vul_arg_parse(argc, argv);

  unsigned iw = arg_iw();
  unsigned ih = arg_ih();
  const unsigned iter = arg_nj();

  if (!arg_use_cpu() && !arg_use_gpu())
  {
    std::cerr << "must specify either -cpu or -gpu flag, or both" << std::endl;
    return -1;
  }

  viscl::manager::inst()->report_opencl_specs();

  vil_image_view<vxl_byte> output, img = vil_image_view<vxl_byte>(iw,ih);
  std::srand(std::time(NULL));
  for(unsigned j=0; j<img.nj(); ++j)
  {
    for(unsigned i=0; i<img.ni(); ++i)
    {
      img(i,j) = std::rand();
    }
  }

  int radii[3] = {2, 3, 4};

  viscl::gaussian_smooth_t smoother = NEW_VISCL_TASK(gaussian_smooth);
  viscl::image img_cl = viscl::upload_image(img);


  for (int r = 0; r < 3; r++)
  {
    boost::chrono::system_clock::time_point start;
    boost::chrono::duration<double> sec;
    if (arg_use_gpu())
    {
      start = boost::chrono::system_clock::now();
      for (int i = 0; i < iter; i++)
        viscl::image result = smoother->smooth( img_cl, 2.0, radii[r]);
      sec = boost::chrono::system_clock::now() - start;
      std::cout << "viscl took " << sec.count() / iter
               << " seconds to smooth a "
               << img.ni() << "x" << img.nj() << " image "
               << iter << "x w/ kernel size=" << 2*radii[r]+1 << ".\n";
    }
    if (arg_use_cpu())
    {
      start = boost::chrono::system_clock::now();
      for (int i = 0; i < iter; i++)
        vil_gauss_filter_2d<vxl_byte, vxl_byte>(img, output, 2.0, radii[r]);
      sec = boost::chrono::system_clock::now() - start;
      std::cout << "VXL took "   << sec.count() / iter
               << " seconds to smooth a "
               << img.ni() << "x" << img.nj() << " img "
               << iter << "x w/ kernel size=" << 2*radii[r]+1 << ".\n";
    }
  }

  return 0;
}
