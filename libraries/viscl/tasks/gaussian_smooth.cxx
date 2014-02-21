/*ckwg +29
 * Copyright 2012 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gaussian_smooth.h"

#include <boost/make_shared.hpp>
#include <viscl/core/program_registry.h>
#include <viscl/core/manager.h>

#include <math.h>


extern const char* gaussian_smooth_source;

namespace viscl
{

//*****************************************************************************

gaussian_smooth::gaussian_smooth()
{
  program = program_registry::inst()->register_program(std::string("gaussian_smooth"),
                                                       gaussian_smooth_source);
  conv_x = make_kernel("convolveHoriz1D");
  conv_y = make_kernel("convolveVert1D");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

image gaussian_smooth::smooth(const image &img, float sigma, int kernel_radius) const
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


  buffer smoothing_kernel = manager::inst()->create_buffer<float>(CL_MEM_READ_ONLY, kernel_size);
  queue->enqueueWriteBuffer(*smoothing_kernel().get(), CL_TRUE, 0, smoothing_kernel.mem_size(), filter);

  size_t ni = img.width(), nj = img.height();
  image working = manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);
  image result = manager::inst()->create_image(img.format(), CL_MEM_READ_WRITE, ni, nj);

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
