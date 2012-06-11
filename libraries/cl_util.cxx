#include "cl_util.h"

#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

template<class T>
void save_cl_image(const cl_queue_t &queue, const cl_image &img, const char *filename)
{
  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(img.ni());
  region.push_back(img.nj());
  region.push_back(1);

  vil_image_view<T> downloaded(img.ni(), img.nj());
  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (float *)downloaded.top_left_ptr());
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

template<>
void save_cl_image<unsigned char>(const cl_queue_t &queue, const cl_image &img, const char *filename)
{
  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(img.ni());
  region.push_back(img.nj());
  region.push_back(1);

  vil_image_view<unsigned char> downloaded(img.ni(), img.nj());
  
  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (float *)downloaded.top_left_ptr());
  vil_save(downloaded, filename);
}

template void save_cl_image<float>(const cl_queue_t &queue, const cl_image &img, const char *filename);
