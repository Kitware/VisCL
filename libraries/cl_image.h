#ifndef CL_IMAGE_H_
#define CL_IMAGE_H_

#include "cl_header.h"
#include <boost/shared_ptr.hpp>

class cl_image
{
public:

  cl_image() {}
  cl_image(const cl_image_t &image);

  size_t ni() const;
  size_t nj() const;

  cl::ImageFormat format() const;
  void del() { img.reset(); }

  const boost::shared_ptr<cl::Image2D> &operator()() const {return img;}

private:

  cl_image_t img;
};

#endif
