/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "conversion.h"

namespace viscl
{

template matrix3x3 vnl_matrix_fixed_3x3_to_viscl_homography(const vnl_matrix_fixed<float, 3, 3> &H);
template matrix3x3 vnl_matrix_fixed_3x3_to_viscl_homography(const vnl_matrix_fixed<double, 3, 3> &H);
template vnl_matrix_fixed<float, 3, 3> viscl_homography_to_vnl_matrix_fixed_3x3(const matrix3x3 &H_cl);
template vnl_matrix_fixed<double, 3, 3> viscl_homography_to_vnl_matrix_fixed_3x3(const matrix3x3 &H_cl);

} // End namespace viscl
