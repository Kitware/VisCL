/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

/* Copy input 2D image to output 2D image */
__kernel void convolveHoriz1D(__read_only image2d_t input, __global const float *filter, int radius, __write_only image2d_t output)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int left = pixel.x - radius;
  int right = pixel.x + radius;

  for(pixel.x =  left; pixel.x <= right; pixel.x++, i++)
  {
    temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }

  write_imagef(output, coord, temp);
}

__kernel void convolveVert1D(__read_only image2d_t input, __global const float *filter, int radius, __write_only image2d_t output)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int top = pixel.y - radius;
  int bottom = pixel.y + radius;

  for(pixel.y =  top; pixel.y <= bottom; pixel.y++, i++)
  {
    temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }

  write_imagef(output, coord, temp);
}
