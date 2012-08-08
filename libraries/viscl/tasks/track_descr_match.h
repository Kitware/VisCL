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

  buffer track(const image &img, int window_size);

  const buffer& last_keypoints() const { return kpts1; }

  int last_num_keypoints() const { return numkpts1; }

  void set_max_kpts(int max) { max_kpts = max; }

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
  cl_queue_t queue;

  buffer kpts1;
  buffer descriptors1;
  image kptmap1;
  int numkpts1, max_kpts;
};

typedef boost::shared_ptr<track_descr_match> track_descr_match_t;

void write_tracks_to_file(const std::string& filename,
                          const std::vector<cl_int2> &kpts1,
                          const std::vector<cl_int2> &kpts2,
                          const std::vector<int> &indices);

}

#endif
