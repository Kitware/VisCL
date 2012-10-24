/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;

typedef struct
{
  float3 row0;
  float3 row1;
  float3 row2;
} homography;


/*****************************************************************************/

//CLK_ADDRESS_CLAMP makes the out of bounds warp val 0,0,0,0 therefore to make use of the hardware image interpolation
//we require that the no-warp value is 0,0,0,0.
__constant int4 nowarp = (int4)(0, 0, 0, 0);

__kernel void init_dest(__write_only image2d_t dest)
{
  write_imagei(dest, (int2)(get_global_id(0), get_global_id(1)), nowarp);
}

/*****************************************************************************/

__kernel void warp_image(__read_only image2d_t src, __write_only image2d_t dest, __global homography *dest_to_src)
{
  int2 dest_pixel = (int2)(get_global_id(0), get_global_id(1));
  float3 dest_pixel_h = (float3)((float)dest_pixel.x, (float)dest_pixel.y, 1.0f);
  float2 src_pixel;
  src_pixel.x = dot(dest_to_src->row0, dest_pixel_h);
  src_pixel.y = dot(dest_to_src->row1, dest_pixel_h);
  src_pixel /= dot(dest_to_src->row2, dest_pixel_h);

  write_imagef(dest, dest_pixel, read_imagef(src, imageSampler, src_pixel));
}
