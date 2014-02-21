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

#ifndef WARP_IMAGE_H_
#define WARP_IMAGE_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/matrix.h>

namespace viscl
{

class warp_image : public task
{
public:

  warp_image();

  ///Warps an image \src using the inverse of the homography \H and stores result in \dest
  ///This function requires dest image to be preallocated.
  void warp(const image &src, const image &dest, const matrix3x3 &H) const;

  ///Warps an image \src using the inverse of the homography \H and stores result in \dest
  ///\left, \top, \right, and \bot are bounds on the warping in \dest image coordinates
  ///This function requires dest image to be preallocated.
  void warp(const image &src, const image &dest, const matrix3x3 &H, unsigned int left,
            unsigned int top, unsigned int right, unsigned int bot) const;

private:

  cl_kernel_t warp_k, init_dest_k;
};

typedef boost::shared_ptr<warp_image> warp_image_t;

}

#endif
