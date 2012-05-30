

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

/* Copy input 2D image to output 2D image */
__kernel void smoothHoriz(__read_only image2d_t input, __global const float *filter, __write_only image2d_t output)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int left = pixel.x - 2;
  int right = pixel.x + 2;

  for(pixel.x =  left; pixel.x <= right; pixel.x++, i++)
  {
    temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }

  write_imagef(output, coord, temp);
}

__kernel void smoothVert(__read_only image2d_t input, __global const float *filter, __write_only image2d_t output)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int top = pixel.y - 2;
  int bottom = pixel.y + 2;

  for(pixel.y =  top; pixel.y <= bottom; pixel.y++, i++)
  {
    temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }

  write_imagef(output, coord, temp);
}
