/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef GAUSSIAN_SMOOTH_H_
#define GAUSSIAN_SMOOTH_H_

#include <viscl/core/cl_task.h>
#include <viscl/core/cl_image.h>

namespace viscl
{


class gaussian_smooth : public cl_task
{
public:

  cl_task_t clone();

  cl_image smooth(const cl_image &img, float sigma, int kernel_radius) const;

protected:

  void init();
  void init(const cl_program_t &prog);

private:

  //This makes it so only the task registry can compile the .cl code
  friend class cl_task_registry;
  gaussian_smooth() {}

  cl_kernel_t conv_x, conv_y;
  cl_queue_t queue;
};

typedef boost::shared_ptr<gaussian_smooth> gaussian_smooth_t;

}

#endif
