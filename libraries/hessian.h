#ifndef HESSIAN_H_
#define HESSIAN_H_

#include "cl_task.h"
#include "cl_image.h"

#include <vil/vil_image_view.h>

class hessian : public cl_task
{
public:

  hessian *clone();

  template <class T>
  void detect(const vil_image_view<T> &img, unsigned int max_keypoints) const;
  cl_image detect(const cl_image &img, unsigned int max_keypoints) const;

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  hessian();

  cl_kernel_t det_hessian, detect_extrema;
  cl_queue_t queue;
};

typedef boost::shared_ptr<hessian> hessian_t;

#endif