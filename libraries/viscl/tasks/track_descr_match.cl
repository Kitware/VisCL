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

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE |
                                    CLK_ADDRESS_CLAMP_TO_EDGE |
                                    CLK_FILTER_NEAREST;

// A Brief descriptor is represented as 128 bits
//using 4 32 bit integers in the int4 datat type
typedef uint4 brief_descr;

// Hamming distance between brief descriptors.
// This function counts the number of bits (pop count)
// between 4 32 ints in parallel then sums the result
uint brief_dist(brief_descr b1, brief_descr b2)
{
  uint4 v = b1 ^ b2; //Find the bits that differ

  //Count the bits in v in parallel
  v = v - ((v >> 1) & 0x55555555);
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
  uint4 c = ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;

  //Sum the counts
  return c.x + c.y + c.z + c.w;
}

__kernel void track(__global    int2        *kpts2,
                    __read_only image2d_t    kptmap1,
                    __global    brief_descr *descriptors1,
                    __global    brief_descr *descriptors2,
                    __global    int         *tracks,
                                unsigned     window,
                                unsigned     closest_dist)
{
  //index into kpts1 and descriptors1
  int index = get_global_id(0);
  brief_descr descr = descriptors2[index];

  //kptmap is 1/2 the size of the original image
  int2 pixel = kpts2[index] / 2;

  int2 loc;
  int closest_index = -1;
  for (loc.x = pixel.x - window; loc.x <= pixel.x + window; loc.x++)
  {
    for (loc.y = pixel.y - window; loc.y <= pixel.y + window; loc.y++)
    {
      int val = read_imagei(kptmap1, imageSampler, loc).x;
      if (val > -1)
      {
        int dist = brief_dist(descriptors1[val], descr);
        if (dist <= closest_dist)
        {
          closest_dist = dist;
          closest_index = val;
        }
      }
    }
  }

  tracks[index] = closest_index;
}
