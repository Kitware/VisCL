#include "gaussian_smooth.h"

#include <boost/make_shared.hpp>
#include <vil/algo/vil_gauss_filter.h>

#include "cl_manager.h"

extern const char* gaussian_smooth_source;

//*****************************************************************************

gaussian_smooth::gaussian_smooth() : cl_task(gaussian_smooth_source)
{
  conv_x = make_kernel("smoothHoriz");
  conv_y = make_kernel("smoothVert");
}

//*****************************************************************************

gaussian_smooth *gaussian_smooth::clone()
{
  gaussian_smooth *clone_ = new gaussian_smooth(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

template <class T>
void gaussian_smooth::smooth(const vil_image_view<T> &img, vil_image_view<T> &output, float sigma) const
{
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_image result = smooth( img_cl, sigma);

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(img.ni());
  region.push_back(img.nj());
  region.push_back(1);

  output.set_size(img.ni(), img.nj());
  queue->enqueueReadImage(*result().get(),  CL_TRUE, origin, region, 0, 0, (float *)output.top_left_ptr());
}

//*****************************************************************************

cl_image gaussian_smooth::smooth(const cl_image &img, float sigma) const
{
  vil_gauss_filter_5tap_params params(sigma);
  float filter[5];
  filter[0] = filter[4] = (float)params.filt2();
  filter[1] = filter[3] = (float)params.filt1();
  filter[2] = (float)params.filt0();

  cl_buffer smoothing_kernel = cl_manager::inst()->create_buffer<float>(CL_MEM_READ_ONLY, 5);
  queue->enqueueWriteBuffer(*smoothing_kernel().get(), CL_TRUE, 0, smoothing_kernel.mem_size(), filter);

  size_t ni = img.ni(), nj = img.nj();
  cl_image working = cl_manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);
  cl_image result = cl_manager::inst()->create_image(img.format(), CL_MEM_WRITE_ONLY, ni, nj);

  // Set arguments to kernel
  conv_x->setArg(0, *img().get());
  conv_x->setArg(1, *smoothing_kernel().get());
  conv_x->setArg(2, *working().get());

  // Set arguments to kernel
  conv_y->setArg(0, *working().get());
  conv_y->setArg(1, *smoothing_kernel().get());
  conv_y->setArg(2, *result().get());

  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  cl::NDRange local(1,1);

  queue->enqueueNDRangeKernel(*conv_x.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueBarrier();
  queue->enqueueNDRangeKernel(*conv_y.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();

  return result;
}

//*****************************************************************************

template void gaussian_smooth::smooth(const vil_image_view<vxl_byte> &img, vil_image_view<vxl_byte> &output, float sigma) const;
template void gaussian_smooth::smooth(const vil_image_view<float> &img, vil_image_view<float> &output, float sigma) const;
