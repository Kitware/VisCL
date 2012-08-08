/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vxl/vxl_cl_util.h"

#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

//Function for debugging, writes out an open cl image to a file
//Scales the value range for visualization purposes
template<class T>
void viscl::save_cl_image(const cl_queue_t &queue, const image &img, const char *filename)
{
  size_t width = img.width();
  size_t height = img.height();

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  vil_image_view<T> downloaded(width, height);
  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (T *)downloaded.top_left_ptr());
  vcl_vector<double> percentiles;
  percentiles.push_back(0.00);
  percentiles.push_back(1.00);
  vcl_vector<T> range;

  vil_math_value_range_percentiles(downloaded, percentiles, range);
  vcl_cout << range[0] << " " << range[1] << "\n";
  double scale = 255.0 / (range[1]-range[0]);
  vil_math_scale_and_offset_values(downloaded, scale, -scale * range[0]);

  vil_image_view<vxl_byte> tosave;
  vil_convert_cast<T, vxl_byte>(downloaded, tosave);
  vil_save(tosave, filename);
}

//Function for debugging, writes out an open cl image to a file
//specilization for unsigned char to not scale image values
template<>
void viscl::save_cl_image<vxl_byte>(const cl_queue_t &queue, const image &img, const char *filename)
{
  size_t width = img.width();
  size_t height = img.height();

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  vil_image_view<vxl_byte> downloaded(width, height);

  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (vxl_byte *)downloaded.top_left_ptr());
  vil_save(downloaded, filename);
}

template void viscl::save_cl_image<float>(const cl_queue_t &queue, const image &img, const char *filename);
