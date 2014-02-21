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

#include "warp_image.h"

#include <viscl/core/program_registry.h>
#include <viscl/core/manager.h>

extern const char* warp_image_source;

namespace viscl
{

//*****************************************************************************

warp_image::warp_image()
{
  program = program_registry::inst()->register_program(std::string("warp_image"),
                                                       warp_image_source);
  warp_k = make_kernel("warp_image");
  init_dest_k = make_kernel("init_dest");
  queue = manager::inst()->create_queue();
}

//*****************************************************************************

void warp_image::warp(const image &src, const image &dest, const matrix3x3 &H) const
{
  warp(src, dest, H, 0, 0, static_cast<unsigned int>(dest.width()), static_cast<unsigned int>(dest.height()));
}

//*****************************************************************************

/// Warp cl image src into cl image dest using the dest-to-src 3x3 homography H
/// left, top, right, bot are guards on the dest image for where the src image will warp
/// This function requires dest image to be preallocated.
void warp_image::warp(const image &src, const image &dest, const matrix3x3 &H, unsigned int left,
                      unsigned int top, unsigned int right, unsigned int bot) const
{
  buffer homog = manager::inst()->create_buffer<matrix3x3>(CL_MEM_READ_ONLY, 1);
  queue->enqueueWriteBuffer(*homog().get(), CL_TRUE, 0, homog.mem_size(), &H);

  init_dest_k->setArg(0, *dest().get());

  // Set arguments to kernel
  warp_k->setArg(0, *src().get());
  warp_k->setArg(1, *dest().get());
  warp_k->setArg(2, *homog().get());

  //Run the kernel on specific ND range
  cl::NDRange init_range(dest.width(), dest.height());
  cl::NDRange warp_range(right - left, bot - top), offset(left, top);

  queue->enqueueNDRangeKernel(*init_dest_k.get(), cl::NullRange, init_range, cl::NullRange);
  queue->enqueueBarrier();
  queue->enqueueNDRangeKernel(*warp_k.get(), offset, warp_range, cl::NullRange);
  queue->finish();
}

//*****************************************************************************

}
