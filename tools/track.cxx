<<<<<<< HEAD
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
=======
/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
>>>>>>> Core builds shared, track can read image list
 */

#include <iostream>
#include <sstream>

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

  std::ifstream imglist(argv[1]);

  try
  {
    viscl::track_descr_match_t tracker = NEW_VISCL_TASK(viscl::track_descr_match);
    tracker->set_search_box_radius(200);
    tracker->set_max_kpts(20000);

    bool first = true;
    int count = 0;
    std::string filename;

    std::vector<cl_int2> *kpts1 = new std::vector<cl_int2>();
    std::vector<cl_int2> *kpts2 = new std::vector<cl_int2>();

    while (imglist >> filename)
    {
      vil_image_view<vxl_byte> img_color = vil_load(filename.c_str());
      vil_image_view<vxl_byte> img;
      vil_convert_planes_to_grey(img_color, img);
      std::vector<cl_int2> kpts;

      if (first)
      {
        std::cout << "start" <<std::endl;
        viscl::track_descr_first_frame(img, *kpts1, tracker);
        first = false;
      }
      else
      {
        std::cout << "tracking " << ++count << "..." << std::endl;
        std::vector<int> indices;
        kpts2->clear();
        indices = viscl::track_descr_track(img, *kpts2, tracker);
        std::ostringstream str;
        str << "tracks_" << count-1 << "-" << count << ".txt";
        viscl::write_tracks_to_file(str.str(), *kpts1, *kpts2, indices);
        std::swap(kpts1, kpts2);
      }
    }

    delete kpts1;
    delete kpts2;
  }
  catch(const cl::Error &e)
  {
    std::cerr << "ERROR: " << e.what() << " (" << e.err() << " : "
             << viscl::print_cl_errstring(e.err()) << ")" << std::endl;
    return 1;
  }
  return 0;
}


