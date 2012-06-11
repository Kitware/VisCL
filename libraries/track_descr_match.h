#ifndef TRACK_NCC_H_
#define TRACK_NCC_H_

#include "cl_task.h"
#include "cl_image.h"

#include "hessian.h"
#include "BRIEF.h"
#include "gaussian_smooth.h"

#include <vil/vil_image_view.h>

class track_descr_match;
typedef boost::shared_ptr<track_descr_match> track_descr_match_t;

class track_descr_match : public cl_task
{
public:

  //Copy constructor for cloning
  track_descr_match(const track_descr_match &t);

  cl_task_t clone();

  template<class T>
  void first_frame(const vil_image_view<T> &img);

  template<class T>
  void track(vil_image_view<T> &img);
  void track(const cl_image &img);

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  track_descr_match();

  hessian_t hes;
  brief<10>::type brf;
  gaussian_smooth_t gs;

  cl_kernel_t track_k;
  cl_queue_t queue;  

  vcl_vector<vcl_vector<cl_int2> > tracks;

  cl_buffer kpts1;
  cl_buffer descriptors1;
  int numkpts1;
};

#endif
