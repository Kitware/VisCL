#include <vcl_iostream.h>

#include <vil/vil_image_view.h>
#include <vil/vil_load.h>
#include <vil/vil_save.h>
#include <vil/vil_convert.h>

#include "cl_manager.h"
#include "cl_task_registry.h"
#include "hessian.h"

int main(int argc, char *argv[])
{
  cl_manager::inst()->report_system_specs();

  vil_image_view<vxl_byte> img_color = vil_load(argv[1]);
  vil_image_view<vxl_byte> img, output;
  vil_convert_planes_to_grey(img_color, img);
  //vcl_cout << print_cl_errstring(-30) << "\n";

  hessian_t gs = NEW_TASK(hessian);
  gs->detect(img, 1000);

  return 0;
}
