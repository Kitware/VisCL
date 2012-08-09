/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include "tasks.h"


template void viscl::cl_hessian_detect(const vil_image_view<float> &img,
                                       int max_kpts, float thresh, float sigma,
                                       vcl_vector<cl_int2> &kpts);
template void viscl::cl_hessian_detect(const vil_image_view<vxl_byte> &img,
                                       int max_kpts, float thresh, float sigma,
                                       vcl_vector<cl_int2> &kpts);

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
                        vcl_vector<cl_int2> &kpts,
                        track_descr_match_t& tdm);
template vcl_vector<int> viscl::
track_descr_track(const vil_image_view<vxl_byte> &img,
                  vcl_vector<cl_int2> &kpts,
                  int window_size,
                  track_descr_match_t& tdm);

//*****************************************************************************

template void viscl::
compute_brief_descriptors<vxl_byte, 10>(const vil_image_view<vxl_byte> &img,
                                        const vcl_vector<cl_int2> &kpts,
                                        vcl_vector<cl_int4> &descriptors,
                                        float sigma);
template void viscl::
compute_brief_descriptors<vxl_byte, 10>(const vil_image_view<vxl_byte> &img,
                                        const vcl_vector<cl_int2> &kpts,
                                        vcl_vector<cl_int4> &descriptors);
