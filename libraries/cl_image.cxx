#include "cl_image.h"

cl_image::cl_image(const boost::shared_ptr<cl::Image2D> &image) : img(image)
{

}

//*****************************************************************************

size_t cl_image::ni() const
{
  size_t width;
  img->getImageInfo<size_t>(CL_IMAGE_WIDTH, &width);
  return width;
}

//*****************************************************************************

size_t cl_image::nj() const
{
  size_t height;
  img->getImageInfo<size_t>(CL_IMAGE_HEIGHT, &height);
  return height;
}

//*****************************************************************************

cl::ImageFormat cl_image::format() const
{
  cl::ImageFormat imgf;
  img->getImageInfo<cl::ImageFormat>(CL_IMAGE_FORMAT, &imgf);
  return imgf;
}

//*****************************************************************************
