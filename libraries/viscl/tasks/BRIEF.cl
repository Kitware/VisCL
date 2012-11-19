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

/* BRIEF dectector
Meta Definitions:
__constant int4 map[128];
*/

typedef uint4 brief_descr;

__constant sampler_t imageSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void brief(__read_only image2d_t input, __global int2 *kpts, __global brief_descr *descr)
{
  int index = get_global_id(0);
  int2 pixel = kpts[index];

  brief_descr d = 0;
  int loc = 0;

  for (int i = 0; i < 32; i++)
  {
    brief_descr less = (uint4)(read_imageui(input, imageSampler, pixel + map[loc].xy).x <
                         read_imageui(input, imageSampler, pixel + map[loc++].zw).x,
                         read_imageui(input, imageSampler, pixel + map[loc].xy).x <
                         read_imageui(input, imageSampler, pixel + map[loc++].zw).x,
                         read_imageui(input, imageSampler, pixel + map[loc].xy).x <
                         read_imageui(input, imageSampler, pixel + map[loc++].zw).x,
                         read_imageui(input, imageSampler, pixel + map[loc].xy).x <
                         read_imageui(input, imageSampler, pixel + map[loc++].zw).x);
    d |= less << i;
  }

  descr[index] = d;
}
