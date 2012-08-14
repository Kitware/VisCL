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

#include <iostream>

#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

#include <viscl/core/manager.h>
#include <viscl/tasks/hessian.h>
#include <viscl/tasks/BRIEF.h>
#include <viscl/tasks/track_descr_match.h>
#include <viscl/vxl/tasks.h>

int main(int argc, char *argv[])
{
  viscl::manager::inst()->report_opencl_specs();

  vil_image_view<vxl_byte> img1_color = vil_load(argv[1]);
  vil_image_view<vxl_byte> img2_color = vil_load(argv[2]);
  vil_image_view<vxl_byte> img3_color = vil_load(argv[3]);

  vil_image_view<vxl_byte> img1, img2, img3, output;
  vil_convert_planes_to_grey(img1_color, img1);
  vil_convert_planes_to_grey(img2_color, img2);
  vil_convert_planes_to_grey(img3_color, img3);

  try
  {
    viscl::track_descr_match_t tracker = NEW_VISCL_TASK(viscl::track_descr_match);
    tracker->set_search_box_radius(200);
    tracker->set_max_kpts(20000);

    std::vector<cl_float2> kpts1, kpts2, kpts3;
    std::cout << "start" <<std::endl;
    viscl::track_descr_first_frame(img1, kpts1, tracker);
    std::cout << "tracked 1" <<std::endl;
    std::vector<int> indices21, indices32;
    indices21 = viscl::track_descr_track(img2, kpts2, tracker);
    std::cout << "tracked 2" <<std::endl;
    indices32 = viscl::track_descr_track(img3, kpts3, tracker);
    std::cout << "tracked 3" <<std::endl;

    viscl::write_tracks_to_file("tracks.txt", kpts2, kpts3, indices32);
  }
  catch(const cl::Error &e)
  {
    std::cerr << "ERROR: " << e.what() << " (" << e.err() << " : "
             << viscl::print_cl_errstring(e.err()) << ")" << std::endl;
    return 1;
  }
  return 0;
}


