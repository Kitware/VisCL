/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_VXL_CONVERSION_H_
#define VISCL_VXL_CONVERSION_H_

#include <vgl/algo/vgl_h_matrix_2d.h>
#include <viscl/core/homography.h>

namespace viscl
{

template <class T>
homography vgl_h_matrix_2d_to_viscl_homography(const vgl_h_matrix_2d<T> &H)
{
  homography H_cl;
  H_cl.row0.s[0] = static_cast<float>(H.get(0,0));
  H_cl.row0.s[1] = static_cast<float>(H.get(0,1));
  H_cl.row0.s[2] = static_cast<float>(H.get(0,2));
  H_cl.row1.s[0] = static_cast<float>(H.get(1,0));
  H_cl.row1.s[1] = static_cast<float>(H.get(1,1));
  H_cl.row1.s[2] = static_cast<float>(H.get(1,2));
  H_cl.row2.s[0] = static_cast<float>(H.get(2,0));
  H_cl.row2.s[1] = static_cast<float>(H.get(2,1));
  H_cl.row2.s[2] = static_cast<float>(H.get(2,2));

  return H_cl;
}


} // End namespace viscl

#endif
