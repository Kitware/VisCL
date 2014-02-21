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
  //printf("%v2f\n", src_pixel);
  write_imagef(dest, dest_pixel, read_imagef(src, imageSampler, src_pixel));
}

/*****************************************************************************/
