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

#ifndef TRACK_DESCR_MATCH_H_
#define TRACK_DESCR_MATCH_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>

#include "hessian.h"
#include "brief.h"
#include "gaussian_smooth.h"

namespace viscl
{

class track_descr_match : public task
{
public:

  track_descr_match();
  ~track_descr_match();

  void first_frame(const image &img);

  buffer track(const image &img);

  const buffer& last_keypoints() const { return kpts1; }

  int last_num_keypoints() const { return numkpts1; }

  /// Set the maximum number of keypoints to track
  void set_max_kpts(int mk) { max_kpts_ = mk; }

  /// Set the search box radius.
  /// The search box is square region with width and height of 2*radius+1.
  void set_search_box_radius(unsigned sbr) { search_box_radius_ = sbr; }

  /// Set the threshold on the maximum hamming distance between descriptors
  void set_hamming_dist_threshold(unsigned hdt) { hamming_dist_threshold_ = hdt; }

  /// Set the Hessian determinant detection threshold
  void set_detect_thresh(float dt) { detect_thresh_ = dt; }

  /// Set the Guassian smoothing sigma
  void set_smooth_sigma(float ss) { smooth_sigma_ = ss; }

  /// Enable/disable sub-pixel interpolation of points
  void set_subpixel(bool sp) { subpixel_ = sp; }

  /// Get the maximum number of keypoints to track
  int max_kpts() const { return max_kpts_; }

  /// Get the search box radius.
  unsigned search_box_radius() const { return search_box_radius_; }

  /// Get the threshold on the maximum hamming distance between descriptors
  unsigned hamming_dist_threshold() const { return hamming_dist_threshold_; }

  /// Get the Hessian determinant detection threshold
  float detect_thresh() const { return detect_thresh_; }

  /// Get the Guassian smoothing sigma
  float smooth_sigma() const { return smooth_sigma_; }

  /// Are the points interpolated to sub-pixel accuracy
  bool subpixel() const { return subpixel_; }

private:

  hessian_t hes;
  brief<10>::type brf;
  gaussian_smooth_t gs;

  cl_kernel_t track_k;

  buffer kpts1;
  buffer descriptors1;
  image kptmap1;
  int numkpts1;

  int max_kpts_;
  unsigned search_box_radius_;
  unsigned hamming_dist_threshold_;
  float detect_thresh_;
  float smooth_sigma_;
  bool subpixel_;
};

typedef boost::shared_ptr<track_descr_match> track_descr_match_t;

void write_tracks_to_file(const std::string& filename,
                          const std::vector<cl_float2> &kpts1,
                          const std::vector<cl_float2> &kpts2,
                          const std::vector<int> &indices);

}

#endif
