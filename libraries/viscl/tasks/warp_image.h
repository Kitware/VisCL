/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef WARP_IMAGE_H_
#define WARP_IMAGE_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/homography.h>

namespace viscl
{

class warp_image : public task
{
public:

  warp_image();

  void warp(const image &src, const image &dest, const homography &H) const;
  void warp(const image &src, const image &dest, const homography &H, unsigned int left,
            unsigned int top, unsigned int right, unsigned int bot) const;

private:

  cl_kernel_t warp_k, init_dest_k;
};

typedef boost::shared_ptr<warp_image> warp_image_t;

}

#endif
