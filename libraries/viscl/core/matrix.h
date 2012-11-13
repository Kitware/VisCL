/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_MATRIX_H_
#define CL_MATRIX_H_

#include <viscl/core/header.h>

namespace viscl
{

class matrix3x3
{
public:
  cl_float3 row0;
  cl_float3 row1;
  cl_float3 row2;
};

}

#endif
