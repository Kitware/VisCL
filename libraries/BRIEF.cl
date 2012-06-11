/* BRIEF dectector 
Meta Definitions:
__const int4 map[128];
*/ 

typedef int4 brief_descr;

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void brief(__read_only image2d_t input, __global int2 *kpts, __global brief_descr *descr)
{
  int index = get_global_id(0);
  int2 pixel = kpts[index];

  brief_descr d = 0;
  int loc = 0;

  for (int i = 0; i < 32; i++)
  {
    int less = read_imageui(input, imageSampler, pixel + map[loc].xy).x <
               read_imageui(input, imageSampler, pixel + map[loc].zw).x;
    d.x |= less << i;
    loc++;
  }

  for (int i = 0; i < 32; i++)
  {
    int less = read_imageui(input, imageSampler, pixel + map[loc].xy).x <
               read_imageui(input, imageSampler, pixel + map[loc].zw).x;
    d.y |= less << i;
    loc++;
  }

  for (int i = 0; i < 32; i++)
  {
    int less = read_imageui(input, imageSampler, pixel + map[loc].xy).x <
               read_imageui(input, imageSampler, pixel + map[loc].zw).x;
    d.z |= less << i;
    loc++;
  }

  for (int i = 0; i < 32; i++)
  {
    int less = read_imageui(input, imageSampler, pixel + map[loc].xy).x <
               read_imageui(input, imageSampler, pixel + map[loc].zw).x;
    d.w |= less << i;
    loc++;
  }

  descr[index] = d;
}

