/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "gaussian_smooth.h"

#include <boost/make_shared.hpp>

#include <viscl/core/cl_manager.h>

#include <math.h>


namespace viscl
{

extern const char* gaussian_smooth_source;

//*****************************************************************************

void gaussian_smooth::init()
{
  cl_task::build_source(gaussian_smooth_source);
  conv_x = make_kernel("convolveHoriz1D");
  conv_y = make_kernel("convolveVert1D");
}

//*****************************************************************************

void gaussian_smooth::init(const cl_program_t &prog)
{
  program = prog;
  conv_x = make_kernel("convolveHoriz1D");
  conv_y = make_kernel("convolveVert1D");
}

//*****************************************************************************

cl_task_t gaussian_smooth::clone()
{
  gaussian_smooth_t clone_ = boost::make_shared<gaussian_smooth>(*this);
  clone_->queue = cl_manager::inst()->create_queue();
  return clone_;
}


//*****************************************************************************

cl_image gaussian_smooth::smooth(const cl_image &img, float sigma, int kernel_radius) const
{
  int kernel_size = 2*kernel_radius+1;
  float *filter = new float[kernel_size];
  int i = 0;
  float sum=0.0f;
  for (float x = -kernel_radius;  x <= kernel_radius; x++, i++)
  {
    filter[i] = exp( (- x * x) / (2.0f * sigma * sigma));
    sum += filter[i];
  }
  for (i = 0; i < kernel_size; ++i)
  {
    filter[i] /= sum;
  }


  cl_buffer smoothing_kernel = cl_manager::inst()->create_buffer<float>(CL_MEM_READ_ONLY, kernel_size);
  queue->enqueueWriteBuffer(*smoothing_kernel().get(), CL_TRUE, 0, smoothing_kernel.mem_size(), filter);

  size_t ni = img.width(), nj = img.height();
  cl_image working = cl_manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);
  cl_image result = cl_manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);

  // Set arguments to kernel
  conv_x->setArg(0, *img().get());
  conv_x->setArg(1, *smoothing_kernel().get());
  conv_x->setArg(2, kernel_radius);
  conv_x->setArg(3, *working().get());

  // Set arguments to kernel
  conv_y->setArg(0, *working().get());
  conv_y->setArg(1, *smoothing_kernel().get());
  conv_y->setArg(2, kernel_radius);
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

}
