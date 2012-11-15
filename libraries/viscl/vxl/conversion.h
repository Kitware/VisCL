/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_VXL_CONVERSION_H_
#define VISCL_VXL_CONVERSION_H_

#include <vnl/vnl_matrix_fixed.h>
#include <viscl/core/matrix.h>

namespace viscl
{

template <class T>
matrix3x3 vnl_to_viscl(const vnl_matrix_fixed<T, 3, 3> &H)
{
  matrix3x3 H_cl;
  H_cl.row0.s[0] = static_cast<float>(H(0,0));
  H_cl.row0.s[1] = static_cast<float>(H(0,1));
  H_cl.row0.s[2] = static_cast<float>(H(0,2));
  H_cl.row1.s[0] = static_cast<float>(H(1,0));
  H_cl.row1.s[1] = static_cast<float>(H(1,1));
  H_cl.row1.s[2] = static_cast<float>(H(1,2));
  H_cl.row2.s[0] = static_cast<float>(H(2,0));
  H_cl.row2.s[1] = static_cast<float>(H(2,1));
  H_cl.row2.s[2] = static_cast<float>(H(2,2));

  return H_cl;
}

template <class T>
vnl_matrix_fixed<T, 3, 3> viscl_to_vnl(const matrix3x3 &H_cl)
{
  vnl_matrix_fixed<T, 3, 3> H;
  H.set(0, 0, static_cast<T>(H_cl.row0.s[0]));
  H.set(0, 1, static_cast<T>(H_cl.row0.s[1]));
  H.set(0, 2, static_cast<T>(H_cl.row0.s[2]));
  H.set(1, 0, static_cast<T>(H_cl.row1.s[0]));
  H.set(1, 1, static_cast<T>(H_cl.row1.s[1]));
  H.set(1, 2, static_cast<T>(H_cl.row1.s[2]));
  H.set(2, 0, static_cast<T>(H_cl.row2.s[0]));
  H.set(2, 1, static_cast<T>(H_cl.row2.s[1]));
  H.set(2, 2, static_cast<T>(H_cl.row2.s[2]));

  return H;
}


} // End namespace viscl

#endif
