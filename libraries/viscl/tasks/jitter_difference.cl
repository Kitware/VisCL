/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

/* Meta Definitions
#define JITTER_DELTA 1
#define JITTER_DELTA_X2 2
#define LOCAL_X 16
#define LOCAL_Y 16
*/
__constant sampler_t image_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

//fetches memory from two images to a local buffer
void fill_local_mem(__read_only image2d_t img1,
                    __read_only image2d_t img2,
                    __local float2 buf[JITTER_DELTA_X2 + LOCAL_Y][JITTER_DELTA_X2 + LOCAL_X],
                    int2 index_g,
                    int2 index_l)
{
  //load upper left
  buf[index_l.y][index_l.x] = (float2)(read_imagef(img1, image_sampler, index_g+(int2)(-JITTER_DELTA,-JITTER_DELTA)).x,
                                       read_imagef(img2, image_sampler, index_g+(int2)(-JITTER_DELTA,-JITTER_DELTA)).x);
  int2 comp = index_l < JITTER_DELTA_X2;

  //load upper right
  if (comp.x) {
    buf[index_l.y][index_l.x + LOCAL_X] = (float2)(read_imagef(img1, image_sampler, index_g+(int2)(LOCAL_X-JITTER_DELTA,-JITTER_DELTA)).x,
                                                   read_imagef(img2, image_sampler, index_g+(int2)(LOCAL_X-JITTER_DELTA,-JITTER_DELTA)).x);
  }

  //load lower left
  if (comp.y) {
    buf[index_l.y + LOCAL_Y][index_l.x] = (float2)(read_imagef(img1, image_sampler, index_g+(int2)(-JITTER_DELTA, LOCAL_Y-JITTER_DELTA)).x,
                                                   read_imagef(img2, image_sampler, index_g+(int2)(-JITTER_DELTA, LOCAL_Y-JITTER_DELTA)).x);
  }

  if (all(comp)) {
    buf[index_l.y + LOCAL_Y][index_l.x + LOCAL_X] = (float2)(read_imagef(img1, image_sampler, index_g+(int2)(LOCAL_X-JITTER_DELTA, LOCAL_Y-JITTER_DELTA)).x,
                                                             read_imagef(img2, image_sampler, index_g+(int2)(LOCAL_X-JITTER_DELTA, LOCAL_Y-JITTER_DELTA)).x);
  }
}

//computes the min and max value over a window of size JITTER_DELTA * 2 + 1
void compute_min_max(__local float2 buf[JITTER_DELTA_X2 + LOCAL_Y][JITTER_DELTA_X2 + LOCAL_X],
                     __local float4 minmax[LOCAL_Y][LOCAL_X],
                     int2 index)
{
  float2 maxv = -INFINITY, minv = INFINITY;

  //offset into jitter buffer
  int2 window_index = index + (int2)(JITTER_DELTA, JITTER_DELTA);
  for (int i = window_index.y - JITTER_DELTA; i <= window_index.y + JITTER_DELTA; i++)
  {
    for (int j = window_index.x - JITTER_DELTA; j <= window_index.x + JITTER_DELTA; j++)
    {
      //compute mins and maxes of the 2 images in parallel
      float2 val = buf[i][j];
      maxv = max(val, maxv);
      minv = min(val, minv);
    }
  }

  minmax[index.y][index.x].xy = minv;
  minmax[index.y][index.x].zw = maxv;
}

//A and B are two previous frames, C is the current frame
//Implements: diff = | jit(| A - C |) + jit(| C - B |) - jit(| A - B |) |
__kernel void jitter_difference(__read_only image2d_t A, __read_only image2d_t B, __read_only image2d_t C,
                                __write_only image2d_t output)
{

  int2 index_g = (int2)(get_global_id(0), get_global_id(1));
  int2 index_l = (int2)(get_local_id(0), get_local_id(1));
  __local float2 buf[JITTER_DELTA_X2 + LOCAL_Y][JITTER_DELTA_X2 + LOCAL_X];

  fill_local_mem(B, C, buf, index_g, index_l);

  barrier(CLK_LOCAL_MEM_FENCE);

  __local float4 minmax[LOCAL_Y][LOCAL_X];
  compute_min_max(buf, minmax, index_l);

  barrier(CLK_LOCAL_MEM_FENCE);

  float a = read_imagef(A, image_sampler, index_g).x;
  float c = buf[index_l.y+JITTER_DELTA][index_l.x+JITTER_DELTA].y;
  float2 bmm = minmax[index_l.y][index_l.x].xz;
  float2 cmm = minmax[index_l.y][index_l.x].yw;

  int2 comp_ac = a < cmm;
  int2 comp_cb = c < bmm;
  int2 comp_ab = a < bmm;

  float diff = 0.0f;
  diff += comp_ac.x ? cmm.x - a : (comp_ac.y ? 0 : a - cmm.y);
  diff += comp_cb.x ? bmm.x - c : (comp_cb.y ? 0 : c - bmm.y);
  diff -= comp_ab.x ? bmm.x - a : (comp_ab.y ? 0 : a - bmm.y);

  write_imagef(output, index_g, fabs(diff));
}
