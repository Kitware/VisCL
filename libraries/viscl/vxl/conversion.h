/*ckwg +29
 * Copyright 2012 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
