
__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void det_hessian(__read_only image2d_t input, __write_only image2d_t output)
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

  float det = H.x * H.y - H.z * H.z;

  write_imagef(output, pixel, (float4)det);
}

__kernel void detect_extrema(__read_only image2d_t detimg, __write_only image2d_t extrema)
{
  int2 pixel = (int2)(get_global_id(0), get_global_id(1));

  unsigned char kpt = 0;
  float val = read_imagef(detimg, imageSampler, pixel).x;

  if (read_imagef(detimg, imageSampler, pixel + (int2)(0,1)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(1,1)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(1,0)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(1,-1)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(0,-1)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,-1)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,0)).x > val &&
      read_imagef(detimg, imageSampler, pixel + (int2)(-1,1)).x > val)
  {
    kpt = 1;
  }

  write_imageui(extrema, pixel, kpt);
}
