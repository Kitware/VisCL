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

#include "util.h"

#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

//Function for debugging, writes out an open cl image to a file
//Scales the value range for visualization purposes
template<class T>
void viscl::save_cl_image(const cl_queue_t &queue, const image &img, const char *filename)
{
  size_t width = img.width();
  size_t height = img.height();

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  vil_image_view<T> downloaded(width, height);
  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (T *)downloaded.top_left_ptr());
  vcl_vector<double> percentiles;
  percentiles.push_back(0.00);
  percentiles.push_back(1.00);
  vcl_vector<T> range;

  vil_math_value_range_percentiles(downloaded, percentiles, range);
  vcl_cout << range[0] << " " << range[1] << "\n";
  double scale = 255.0 / (range[1]-range[0]);
  vil_math_scale_and_offset_values(downloaded, scale, -scale * range[0]);

  vil_image_view<vxl_byte> tosave;
  vil_convert_cast<T, vxl_byte>(downloaded, tosave);
  vil_save(tosave, filename);
}

//Function for debugging, writes out an open cl image to a file
//specilization for unsigned char to not scale image values
template<>
void viscl::save_cl_image<vxl_byte>(const cl_queue_t &queue, const image &img, const char *filename)
{
  size_t width = img.width();
  size_t height = img.height();

  cl::size_t<3> origin;
  origin.push_back(0);
  origin.push_back(0);
  origin.push_back(0);

  cl::size_t<3> region;
  region.push_back(width);
  region.push_back(height);
  region.push_back(1);

  vil_image_view<vxl_byte> downloaded(width, height);

  queue->enqueueReadImage(*img().get(),  CL_TRUE, origin, region, 0, 0, (vxl_byte *)downloaded.top_left_ptr());
  vil_save(downloaded, filename);
}

template void viscl::save_cl_image<float>(const cl_queue_t &queue, const image &img, const char *filename);
