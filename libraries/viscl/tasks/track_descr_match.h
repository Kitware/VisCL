/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef TRACK_DESCR_MATCH_H_
#define TRACK_DESCR_MATCH_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>

#include "hessian.h"
#include "BRIEF.h"
#include "gaussian_smooth.h"


namespace viscl
{

class track_descr_match : public task
{
public:

  ~track_descr_match();

  //Copy constructor for cloning
  track_descr_match(const track_descr_match &t);
  task_t clone();

  void first_frame(const image &img);

  buffer track(const image &img);

  const buffer& last_keypoints() const { return kpts1; }

  int last_num_keypoints() const { return numkpts1; }

  /// Set the maximum number of keypoints to track
  void set_max_kpts(int mk) { max_kpts_ = mk; }

  /// Set the search box radius.
  /// The search box is square region with width and height of 2*radius+1.
  void set_search_box_radius(unsigned sbr) { search_box_radius_ = sbr; }

  /// Set the Hessian determinant detection threshold
  void set_detect_thresh(float dt) { detect_thresh_ = dt; }

  /// Set the Guassian smoothing sigma
  void set_smooth_sigma(float ss) { smooth_sigma_ = ss; }


  /// Get the maximum number of keypoints to track
  int max_kpts() const { return max_kpts_; }

  /// Get the search box radius.
  unsigned search_box_radius() const { return search_box_radius_; }

  /// Get the Hessian determinant detection threshold
  float detect_thresh() const { return detect_thresh_; }

  /// Get the Guassian smoothing sigma
  float smooth_sigma() const { return smooth_sigma_; }

protected:

  void init();
  void init(const cl_program_t &prog);

private:

  //This makes it so only the task registry can compile the .cl code
  friend class task_registry;
  track_descr_match();

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
  float detect_thresh_;
  float smooth_sigma_;
};

typedef boost::shared_ptr<track_descr_match> track_descr_match_t;

void write_tracks_to_file(const std::string& filename,
                          const std::vector<cl_int2> &kpts1,
                          const std::vector<cl_int2> &kpts2,
                          const std::vector<int> &indices);

}

#endif
