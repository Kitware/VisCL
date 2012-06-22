/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef TRACK_DESCR_MATCH_H_
#define TRACK_DESCR_MATCH_H_

#include "cl_task.h"
#include "cl_image.h"

#include "hessian.h"
#include "BRIEF.h"
#include "gaussian_smooth.h"

#include <vil/vil_image_view.h>
#include <vnl/vnl_int_2.h>

class track_descr_match;

typedef boost::shared_ptr<track_descr_match> track_descr_match_t;

class track_descr_match : public cl_task
{
public:

  struct pt_track
  {
    vnl_int_2 pt_prev;
    vnl_int_2 pt_new;
  };

  //Copy constructor for cloning
  track_descr_match(const track_descr_match &t);
  cl_task_t clone();

  template<class T>
  void first_frame(const vil_image_view<T> &img);

  template<class T>
  vcl_vector<pt_track> track(const vil_image_view<T> &img, int window_size);

  void write_tracks_to_file(const char *filename);
  //void track(const cl_image &img, int window_size);

  const vcl_vector<pt_track> &get_tracks() const { return tracks; }
  void set_max_kpts(int max) { max_kpts = max; }

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  track_descr_match();

  hessian_t hes;
  brief<10>::type brf;
  gaussian_smooth_t gs;

  cl_kernel_t track_k;
  cl_queue_t queue;  

  vcl_vector<pt_track> tracks;

  cl_buffer kpts1;
  cl_buffer descriptors1;
  int numkpts1, max_kpts;
};

typedef track_descr_match::pt_track tdm_track;

#endif
