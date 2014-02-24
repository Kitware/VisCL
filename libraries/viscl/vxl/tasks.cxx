/*ckwg +29
 * Copyright 2012-2014 by Kitware, Inc.
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


#include "tasks.h"


template void viscl::cl_hessian_detect(const vil_image_view<float> &img,
                                       float thresh, float sigma,
                                       vcl_vector<cl_float2> &kpts,
                                       vcl_vector<float> &kvals);
template void viscl::cl_hessian_detect(const vil_image_view<vxl_byte> &img,
                                       float thresh, float sigma,
                                       vcl_vector<cl_float2> &kpts,
                                       vcl_vector<float> &kvals);

//*****************************************************************************

template void viscl::cl_gaussian_smooth(const vil_image_view<vxl_byte> &,
                                        vil_image_view<vxl_byte> &,
                                        float, int);
template void viscl::cl_gaussian_smooth(const vil_image_view<float> &,
                                        vil_image_view<float> &,
                                        float, int);

//*****************************************************************************

template void viscl::
track_descr_first_frame(const vil_image_view<vxl_byte> &img,
                        vcl_vector<cl_float2> &kpts,
                        track_descr_match_t& tdm);
template vcl_vector<int> viscl::
track_descr_track(const vil_image_view<vxl_byte> &img,
                  vcl_vector<cl_float2> &kpts,
                  track_descr_match_t& tdm);

//*****************************************************************************

template void viscl::
compute_brief_descriptors<vxl_byte, 10>(const vil_image_view<vxl_byte> &img,
                                        const vcl_vector<cl_float2> &kpts,
                                        vcl_vector<cl_int4> &descriptors,
                                        float sigma);
template void viscl::
compute_brief_descriptors<vxl_byte, 10>(const vil_image_view<vxl_byte> &img,
                                        const vcl_vector<cl_float2> &kpts,
                                        vcl_vector<cl_int4> &descriptors);

//*****************************************************************************

template bool viscl::
warp_image_vxl(const vil_image_view<float> &src, vil_image_view<float> &dest, const vgl_h_matrix_2d<float> &H);
template bool viscl::
warp_image_vxl(const vil_image_view<vxl_byte> &src, vil_image_view<vxl_byte> &dest, const vgl_h_matrix_2d<float> &H);
template bool viscl::
warp_image_vxl(const vil_image_view<vxl_byte> &src, vil_image_view<vxl_byte> &dest, const vgl_h_matrix_2d<double> &H);

template bool viscl::
warp_image_vxl(const vil_image_view<float> &src, viscl::image &dest, const size_t dni, const size_t dnj,
               const vgl_h_matrix_2d<float> &H, const viscl::warp_image_t &warper);
template bool viscl::
warp_image_vxl(const vil_image_view<vxl_byte> &src, viscl::image &dest, const size_t dni, const size_t dnj,
               const vgl_h_matrix_2d<float> &H, const viscl::warp_image_t &warper);
template bool viscl::
warp_image_vxl(const vil_image_view<vxl_byte> &src, viscl::image &dest, const size_t dni, const size_t dnj,
               const vgl_h_matrix_2d<double> &H, const viscl::warp_image_t &warper);

//*****************************************************************************
