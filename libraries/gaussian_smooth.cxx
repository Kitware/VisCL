/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "gaussian_smooth.h"

#include <boost/make_shared.hpp>

#include "cl_manager.h"

extern const char* gaussian_smooth_source;

//*****************************************************************************

void gaussian_smooth::init()
{
  cl_task::build_source(gaussian_smooth_source);
  conv_x = make_kernel("smoothHoriz");
  conv_y = make_kernel("smoothVert");
}

//*****************************************************************************

void gaussian_smooth::init(const cl_program_t &prog)
{
  program = prog;
  conv_x = make_kernel("smoothHoriz");
  conv_y = make_kernel("smoothVert");
}

//*****************************************************************************

cl_task_t gaussian_smooth::clone()
{
  gaussian_smooth_t clone_ = boost::make_shared<gaussian_smooth>(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}

//*****************************************************************************

template <class T>
void gaussian_smooth::smooth(const vil_image_view<T> &img, vil_image_view<T> &output, float sigma, int kernel_radius) const
{
  cl_image img_cl = cl_manager::inst()->create_image<T>(img);
  cl_image result = smooth( img_cl, sigma, kernel_radius);

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

cl_image gaussian_smooth::smooth(const cl_image &img, float sigma, int kernel_radius) const
{
  int kernel_size = 2*kernel_radius+1;
  float *filter = new float[kernel_size];
  float coeff = 1.0f / sqrt(6.2831853072f * sigma * sigma);
  int i = 0;
  for (float x = -kernel_radius;  x <= kernel_radius; x++, i++)
  {
    filter[i] = coeff * exp( (- x * x) / (2.0f * sigma * sigma));
  }

  cl_buffer smoothing_kernel = cl_manager::inst()->create_buffer<float>(CL_MEM_READ_ONLY, 5);
  queue->enqueueWriteBuffer(*smoothing_kernel().get(), CL_TRUE, 0, smoothing_kernel.mem_size(), filter);

  size_t ni = img.ni(), nj = img.nj();
  cl_image working = cl_manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);
  cl_image result = cl_manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);

  // Set arguments to kernel
  conv_x->setArg(0, *img().get());
  conv_x->setArg(1, *smoothing_kernel().get());
  conv_x->setArg(2, kernel_size);
  conv_x->setArg(3, *working().get());

  // Set arguments to kernel
  conv_y->setArg(0, *working().get());
  conv_y->setArg(1, *smoothing_kernel().get());
  conv_y->setArg(2, kernel_size);
  conv_y->setArg(3, *result().get());

  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  //cl::NDRange local(1,1);

  queue->enqueueNDRangeKernel(*conv_x.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueBarrier();
  queue->enqueueNDRangeKernel(*conv_y.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();

  delete [] filter;

  return result;
}

//*****************************************************************************

template void gaussian_smooth::smooth(const vil_image_view<vxl_byte> &, vil_image_view<vxl_byte> &, float, int) const;
template void gaussian_smooth::smooth(const vil_image_view<float> &, vil_image_view<float> &, float, int) const;
