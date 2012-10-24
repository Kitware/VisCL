/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "conversion.h"

namespace viscl
{

template homography vgl_h_matrix_2d_to_viscl_homography(const vgl_h_matrix_2d<float> &H);
template homography vgl_h_matrix_2d_to_viscl_homography(const vgl_h_matrix_2d<double> &H);

} // End namespace viscl
