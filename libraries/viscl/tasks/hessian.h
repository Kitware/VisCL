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

#ifndef HESSIAN_H_
#define HESSIAN_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/buffer.h>


namespace viscl
{

class hessian : public task
{
public:

  hessian();

  void smooth_and_detect(const image &img, image &kptmap, buffer &kpts,
                         buffer &kvals, buffer &numkpts,
                         float thresh, float sigma, bool subpixel = false) const;

  /// Detect the Hessian determinant keypoints.
  /// \param img input image
  /// \param kptmap output keypoint map image (half size of input)
  ///               caching keypoint indices by location/2
  /// \param kpts buffer of detect keypoint coordinates (float2)
  /// \param kvals buffer of magnitudes of each keypoint detection (float)
  /// \param numkpts a buffer containing the number of keypoints detected.
  /// \param thresh detection threshold on determinant of Hessian.
  /// \param scale the scale (Gaussian sigma) of these points.
  /// \param subpixel if true, compute sub-pixel interpolated keypoints
  void detect(const image &img, image &kptmap, buffer &kpts, buffer &kvals,
              buffer &numkpts, float thresh, float scale,
              bool subpixel = false) const;

  unsigned num_kpts(const buffer &numkpts_b) const;

private:



  cl_kernel_t det_hessian;
  cl_kernel_t detect_extrema;
  cl_kernel_t detect_extrema_subpix;
  cl_kernel_t init_kpt_map;
  mutable unsigned kpts_buffer_size_;
};

typedef boost::shared_ptr<hessian> hessian_t;

}

#endif
