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

#include "hessian.h"

#include <boost/make_shared.hpp>

#include <viscl/core/program_registry.h>
#include <viscl/core/manager.h>

#include "gaussian_smooth.h"

extern const char* hessian_source;

namespace viscl
{

//*****************************************************************************

hessian::hessian()
{
  program = program_registry::inst()->register_program(std::string("hessian"), hessian_source);
  det_hessian = make_kernel("det_hessian");
  detect_extrema = make_kernel("detect_extrema");
  init_kpt_map = make_kernel("init_kpt_map");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

void hessian::smooth_and_detect(const image &img, image &kptmap, buffer &kpts, buffer &numkpts,
                         int max_kpts, float thresh, float sigma) const
{
  float scale = 2.0f;
  gaussian_smooth_t gs = NEW_VISCL_TASK(viscl::gaussian_smooth);
  image smoothed = gs->smooth(img, scale, 2);
  detect(smoothed, kptmap, kpts, numkpts, max_kpts, thresh, sigma);
}

//*****************************************************************************

void hessian::detect(const image &smoothed, image &kptmap, buffer &kpts, buffer &numkpts,
                     int max_kpts, float thresh, float sigma) const
{
  size_t ni = smoothed.width(), nj = smoothed.height();
  cl::ImageFormat detimg_fmt(CL_INTENSITY, CL_FLOAT);
  image detimg = manager::inst()->create_image(detimg_fmt, CL_MEM_READ_WRITE, ni, nj);
  cl::ImageFormat kptimg_fmt(CL_R, CL_SIGNED_INT32);
  kptmap = manager::inst()->create_image(kptimg_fmt, CL_MEM_READ_WRITE, ni >> 1, nj >> 1);
  kpts = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_WRITE, max_kpts);

  int init[1];
  init[0] = 0;
  numkpts = manager::inst()->create_buffer<int>(CL_MEM_READ_WRITE, 1);
  queue->enqueueWriteBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), init);

  // Set arguments to kernel
  det_hessian->setArg(0, *smoothed().get());
  det_hessian->setArg(1, *detimg().get());
  det_hessian->setArg(2, sigma*sigma);

  init_kpt_map->setArg(0, *kptmap().get());

  // Set arguments to kernel
  detect_extrema->setArg(0, *detimg().get());
  detect_extrema->setArg(1, *kptmap().get());
  detect_extrema->setArg(2, *kpts().get());
  detect_extrema->setArg(3, *numkpts().get());
  detect_extrema->setArg(4, max_kpts);
  detect_extrema->setArg(5, thresh);

  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  cl::NDRange initsize(ni >> 1, nj >> 1);
  //cl::NDRange local(32,32);

  queue->enqueueNDRangeKernel(*det_hessian.get(), cl::NullRange, global, cl::NullRange);
  queue->enqueueNDRangeKernel(*init_kpt_map.get(), cl::NullRange, initsize, cl::NullRange);
  queue->enqueueBarrier();

  queue->enqueueNDRangeKernel(*detect_extrema.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();
}

//*****************************************************************************

int hessian::num_kpts(const buffer &numkpts_b)
{
  int buf[1];
  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  return buf[0];
}

}
