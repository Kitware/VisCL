#ifndef GAUSSIAN_SMOOTH_H_
#define GAUSSIAN_SMOOTH_H_

#include "cl_task.h"

#include <vil/vil_image_view.h>

class gaussian_smooth : public cl_task
{
public:

  gaussian_smooth();

  template<class T>
  void smooth(vil_image_view<T> &img, float sigma);

  template<>
  void smooth(vil_image_view<vxl_byte> &img, float sigma);

  void smooth(cl::Image2D *img, float sigma);


private:

};

#endif