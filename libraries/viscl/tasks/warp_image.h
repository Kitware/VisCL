/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef WARP_IMAGE_H_
#define WARP_IMAGE_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/matrix.h>

namespace viscl
{

class warp_image : public task
{
public:

  warp_image();

  ///Warps an image \src using the inverse of the homography \H and stores result in \dest
  ///This function requires dest image to be preallocated.
  void warp(const image &src, const image &dest, const matrix3x3 &H) const;

  ///Warps an image \src using the inverse of the homography \H and stores result in \dest
  ///\left, \top, \right, and \bot are bounds on the warping in \dest image coordinates
  ///This function requires dest image to be preallocated.
  void warp(const image &src, const image &dest, const matrix3x3 &H, unsigned int left,
            unsigned int top, unsigned int right, unsigned int bot) const;

private:

  cl_kernel_t warp_k, init_dest_k;
};

typedef boost::shared_ptr<warp_image> warp_image_t;

}

#endif
