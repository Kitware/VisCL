
__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void det_hessian(__read_only image2d_t input, __write_only image2d_t output, float scale2)
{
  int2 pixel = (int2)(get_global_id(0), get_global_id(1));
  float3 H = 0.0f;

  float center2 = 2*read_imagef(input, imageSampler, pixel).x;
  H.x = read_imagef(input, imageSampler, pixel + (int2)(1,0)).x - center2 +
        read_imagef(input, imageSampler, pixel + (int2)(-1,0)).x;
  H.y = read_imagef(input, imageSampler, pixel + (int2)(0,1)).x - center2 +
        read_imagef(input, imageSampler, pixel + (int2)(0,-1)).x;
  H.z = (read_imagef(input, imageSampler, pixel + (int2)(1,1)).x -
        read_imagef(input, imageSampler, pixel + (int2)(1,-1)).x -
        read_imagef(input, imageSampler, pixel + (int2)(-1,1)).x +
        read_imagef(input, imageSampler, pixel + (int2)(-1,-1)).x)/4.0f;

  float det = scale2 * (H.x * H.y - H.z * H.z);

  write_imagef(output, pixel, (float4)det);
}

/*****************************************************************************/

__constant int4 noval = (int4)(-1, -1, -1, -1);

__kernel void init_kpt_map(__write_only image2d_t kptmap)
{
  write_imagei(kptmap, (int2)(get_global_id(0), get_global_id(1)), noval);
}

/*****************************************************************************/

__kernel void detect_extrema(__read_only image2d_t detimg, __write_only image2d_t kptmap, __global int2 *kpts, __global int *numkpts, int max_kpts, float thresh)
{
  if (numkpts[0] >= max_kpts)
  {
    return;
  }

  int2 pixel = (int2)(get_global_id(0), get_global_id(1));
  float val = read_imagef(detimg, imageSampler, pixel).x;
  if (val < thresh)
  {
    return;
  }

  if (read_imagef(detimg, imageSampler, pixel + (int2)(0,1)).x > val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,1)).x > val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,0)).x > val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,-1)).x > val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(0,-1)).x > val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,-1)).x > val ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,0)).x > val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,1)).x > val)
  {
    return;
  }

  int index = atomic_add(numkpts, 1);
  if (index < max_kpts)
  {
    kpts[index] = pixel;
    int2 mappixel = pixel >> 1;
    write_imagei(kptmap, mappixel, index);
  }
  else
  {
    numkpts[0] = max_kpts;
  }
}
