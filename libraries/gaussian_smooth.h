#ifndef GAUSSIAN_SMOOTH_H_
#define GAUSSIAN_SMOOTH_H_

#include "cl_task.h"
#include "cl_image.h"

#include <vil/vil_image_view.h>

class gaussian_smooth : public cl_task
{
public:

  cl_task_t clone();

  template<class T>
  void smooth(const vil_image_view<T> &img, vil_image_view<T> &output, float sigma) const;
  cl_image smooth(const cl_image &img, float sigma) const;

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  gaussian_smooth();

  cl_kernel_t conv_x, conv_y;
  cl_queue_t queue;
};

typedef boost::shared_ptr<gaussian_smooth> gaussian_smooth_t;

#endif
