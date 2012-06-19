#include <vcl_iostream.h>

#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

#include "cl_manager.h"
#include "cl_task_registry.h"
#include "hessian.h"
#include "BRIEF.h"
#include "track_descr_match.h"

int main(int argc, char *argv[])
{
  cl_manager::inst()->report_system_specs();

  vil_image_view<vxl_byte> img1_color = vil_load(argv[1]);
  vil_image_view<vxl_byte> img2_color = vil_load(argv[2]);
  vil_image_view<vxl_byte> img1, img2, output;
  vil_convert_planes_to_grey(img1_color, img1);
  vil_convert_planes_to_grey(img2_color, img2);

  //vcl_cout << print_cl_errstring(-30) << "\n";

  //hessian_t gs = NEW_VISCL_TASK(hessian);

  //vcl_vector<cl_int2> kpts;
  //gs->detect(img, 10000, 0.01f, 2.0f, kpts);
  ////vcl_ofstream outfile("kpts.txt");
  ////for (unsigned int i = 0; i < kpts.size(); i++)
  ////{
  ////  outfile << kpts[i].s[0] << " " << kpts[i].s[1] << "\n";
  ////}
  ////outfile.close();

  //brief<10>::type br = NEW_VISCL_TASK(brief<10>);
  //vcl_vector<cl_int4> descriptors;
  //br->compute_descriptors(img, kpts, descriptors, 2.0f);

  track_descr_match_t tracker = NEW_VISCL_TASK(track_descr_match);
  vcl_cout << "start\n";
  tracker->first_frame(img1);
  tracker->track(img2);
  

  return 0;
}


