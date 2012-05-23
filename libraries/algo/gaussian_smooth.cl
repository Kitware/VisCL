#ifndef SCALE_SPACE_CL_
#define SCALE_SPACE_CL_

#define STRINGIFY(A) #A

const char *gaussian_smooth_source = STRINGIFY(

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

/* Copy input 2D image to output 2D image */
__kernel void conv_x(__read_only image2d_t input, __global const float *filter, __write_only image2d_t output, uint2 dims)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int left = pixel.x - 2;
  int right = pixel.x + 2;

  for(pixel.x =  left; pixel.x <= right; pixel.x++, i++)
  {
    if (pixel.x >= 0 && pixel.x < dims.x)
      temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }
  
  write_imagef(output, coord, temp);
}

__kernel void conv_y(__read_only image2d_t input, __global const float *filter, __write_only image2d_t output, uint2 dims)
{
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  float4 temp = 0.0f;
  int i = 0;

  int2 pixel = coord;
  int top = pixel.y - 2;
  int bottom = pixel.y + 2;

  for(pixel.y =  top; pixel.y <= bottom; pixel.y++, i++)
  {
    if (pixel.y >= 0 && pixel.y < dims.y)
      temp +=  filter[i] * read_imagef(input, imageSampler, pixel);
  }
  
  write_imagef(output, coord, temp);
}

);

#endif
