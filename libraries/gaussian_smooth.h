/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
  void smooth(const vil_image_view<T> &img, vil_image_view<T> &output, float sigma, int kernel_radius) const;
  cl_image smooth(const cl_image &img, float sigma, int kernel_radius) const;

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  gaussian_smooth();

  cl_kernel_t conv_x, conv_y;
  cl_queue_t queue;
};

typedef boost::shared_ptr<gaussian_smooth> gaussian_smooth_t;

#endif
