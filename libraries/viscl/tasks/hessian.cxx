/*ckwg +29
 * Copyright 2012-2014 by Kitware, Inc.
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

#include <algorithm>
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
  detect_extrema_subpix = make_kernel("detect_extrema_subpix");
  init_kpt_map = make_kernel("init_kpt_map");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

void hessian::det_hessian_image(const image &img, image &detimg, float scale) const
{
  const size_t ni = img.width(), nj = img.height();

  // allocate a new image if the current one is not the right size and type
  cl::ImageFormat detimg_fmt(CL_INTENSITY, CL_FLOAT);
  if ( detimg.width() != ni ||
       detimg.height() != nj ||
       detimg.format().image_channel_data_type != detimg_fmt.image_channel_data_type ||
       detimg.format().image_channel_order != detimg_fmt.image_channel_order )
  {
    detimg = manager::inst()->create_image(detimg_fmt, CL_MEM_READ_WRITE, ni, nj);
  }

  // Set arguments to kernel
  det_hessian->setArg(0, *img().get());
  det_hessian->setArg(1, *detimg().get());
  det_hessian->setArg(2, scale*scale);

  //Run the kernel on specific ND range
  cl::NDRange global(ni, nj);
  queue->enqueueNDRangeKernel(*det_hessian.get(), cl::NullRange, global, cl::NullRange);
  queue->finish();
}

//*****************************************************************************

void hessian::find_peaks(const image &resp_img, image &kptmap,
                         buffer &kpts, buffer &kvals, buffer &numkpts,
                         float thresh, bool subpixel) const
{
  const size_t ni = resp_img.width(), nj = resp_img.height();
  // a hard upper bound on the number of keypoints that can be detected
  const unsigned max_kpts = ni * nj / 4;

  // Allocate buffers with an initial guess of size if unallocated
  // If buffers have different size, use the smallest
  unsigned max_detections = std::min(kpts.len(), kvals.len());
  if( max_detections == 0 )
  {
    // a default guess for the total number of keypoints
    max_detections = max_kpts / 100;
    kpts = manager::inst()->create_buffer<cl_float2>(CL_MEM_READ_WRITE, max_detections);
    kvals = manager::inst()->create_buffer<cl_float>(CL_MEM_READ_WRITE, max_detections);
  }

  cl::ImageFormat kptimg_fmt(CL_R, CL_SIGNED_INT32);
  kptmap = manager::inst()->create_image(kptimg_fmt, CL_MEM_READ_WRITE, ni >> 1, nj >> 1);
  cl_kernel_t extrema = subpixel ? detect_extrema_subpix : detect_extrema;

  int init[1];
  init[0] = 0;
  numkpts = manager::inst()->create_buffer<int>(CL_MEM_READ_WRITE, 1);
  queue->enqueueWriteBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), init);

  init_kpt_map->setArg(0, *kptmap().get());

  // Set arguments to kernel
  extrema->setArg(0, *resp_img().get());
  extrema->setArg(1, *kptmap().get());
  extrema->setArg(2, *kpts().get());
  extrema->setArg(3, *kvals().get());
  extrema->setArg(4, max_detections);
  extrema->setArg(5, *numkpts().get());
  extrema->setArg(6, thresh);

  //Run the kernel on specific ND range
  cl::NDRange global(ni-2, nj-2);
  cl::NDRange offset(1, 1);
  cl::NDRange initsize(ni >> 1, nj >> 1);
  //cl::NDRange local(32,32);

  queue->enqueueNDRangeKernel(*init_kpt_map.get(), cl::NullRange, initsize, cl::NullRange);
  queue->enqueueBarrier();

  queue->enqueueNDRangeKernel(*extrema.get(), offset, global, cl::NullRange);
  queue->enqueueBarrier();
  unsigned num_detected = this->num_kpts(numkpts);
  // if the keypoint buffer was too small, we need to allocate more memory and try again
  if (num_detected >= max_detections)
  {
    queue->enqueueWriteBuffer(*numkpts().get(), CL_TRUE, 0, numkpts.mem_size(), init);
    if (num_detected > kpts.len())
    {
      kpts = manager::inst()->create_buffer<cl_float2>(CL_MEM_READ_WRITE, num_detected);
    }
    if (num_detected > kvals.len())
    {
      kvals = manager::inst()->create_buffer<cl_float>(CL_MEM_READ_WRITE, num_detected);
    }
    extrema->setArg(2, *kpts().get());
    extrema->setArg(3, *kvals().get());
    extrema->setArg(4, num_detected);
    queue->enqueueNDRangeKernel(*init_kpt_map.get(), cl::NullRange, initsize, cl::NullRange);
    queue->enqueueBarrier();
    queue->enqueueNDRangeKernel(*extrema.get(), offset, global, cl::NullRange);
    queue->finish();
  }
}

//*****************************************************************************

void hessian::smooth_and_detect(const image &img, image &kptmap, buffer &kpts,
                                buffer &kvals, buffer &numkpts,
                                float thresh, float sigma, bool subpixel) const
{
  float scale = 2.0f;
  gaussian_smooth_t gs = NEW_VISCL_TASK(viscl::gaussian_smooth);
  image smoothed = gs->smooth(img, scale, 2);
  detect(smoothed, kptmap, kpts, kvals, numkpts, thresh, sigma, subpixel);
}

//*****************************************************************************

void hessian::detect(const image &smoothed, image &kptmap, buffer &kpts,
                     buffer &kvals, buffer &numkpts,
                     float thresh, float sigma, bool subpixel) const
{
  // Compute the image of hessian determinants
  image detimg;
  det_hessian_image(smoothed, detimg, sigma);

  // Find the peaks
  find_peaks(detimg, kptmap, kpts, kvals, numkpts, thresh, subpixel);
}

//*****************************************************************************

unsigned hessian::num_kpts(const buffer &numkpts_b) const
{
  int buf[1];
  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  return buf[0];
}

}
