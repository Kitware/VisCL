/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef GAUSSIAN_SMOOTH_H_
#define GAUSSIAN_SMOOTH_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>

namespace viscl
{

class gaussian_smooth : public task
{
public:

  gaussian_smooth();
  image smooth(const image &img, float sigma, int kernel_radius) const;

protected:

  void init();
  void init(const cl_program_t &prog);

private:

  cl_kernel_t conv_x, conv_y;
};

typedef boost::shared_ptr<gaussian_smooth> gaussian_smooth_t;

}

#endif
