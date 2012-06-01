
__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void det_hessian(__read_only image2d_t input, __write_only image2d_t output, float scale2)
{
  int2 pixel = (int2)(get_global_id(0), get_global_id(1));
  float4 H = 0.0f;

  float center2 = 2*read_imagef(input, imageSampler, pixel).x;
  H.x = read_imagef(input, imageSampler, pixel + (int2)(1,0)).x - center2 +
        read_imagef(input, imageSampler, pixel + (int2)(-1,0)).x;
  H.y = read_imagef(input, imageSampler, pixel + (int2)(0,1)).x - center2 +
        read_imagef(input, imageSampler, pixel + (int2)(0,-1)).x;
  H.z = (read_imagef(input, imageSampler, pixel + (int2)(1,1)).x -
        read_imagef(input, imageSampler, pixel + (int2)(1,-1)).x -
        read_imagef(input, imageSampler, pixel + (int2)(-1,1)).x +
        read_imagef(input, imageSampler, pixel + (int2)(-1,-1)).x)/4;

  float det = scale2 * (H.x * H.y - H.z * H.z);

  write_imagef(output, pixel, (float4)det);
}

__kernel void detect_extrema(__read_only image2d_t detimg, int max_keypoints, float thresh, __global int *count, __global int2 *kpts)
{
  if (count[0] >= max_keypoints)
  {
    return;
  }

  int2 pixel = (int2)(get_global_id(0), get_global_id(1));

  float val = read_imagef(detimg, imageSampler, pixel).x;
  if (val < thresh)
  {
    return;
  }

  if (read_imagef(detimg, imageSampler, pixel + (int2)(0,1)).x <= val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,1)).x <= val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,0)).x <= val   ||
      read_imagef(detimg, imageSampler, pixel + (int2)(1,-1)).x <= val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(0,-1)).x <= val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,-1)).x <= val ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,0)).x <= val  ||
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,1)).x <= val)
  {
    return;
  }

  int loc = atomic_add(count, 1);
  if (loc < max_keypoints)
  {
    kpts[loc] = pixel;
  }
  else
  {
    count[0] = max_keypoints;
  }
}
