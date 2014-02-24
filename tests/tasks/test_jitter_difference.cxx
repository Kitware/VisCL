/*ckwg +29
 * Copyright 2012-2014 by Kitware, Inc.
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


#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <exception>

#include <test_common.h>

#include <viscl/core/manager.h>

#include <viscl/tasks/jitter_difference.h>

#include <boost/chrono.hpp>

#ifdef HAS_VXL
#include <viscl/vxl/transfer.h>
#include <viscl/vxl/tasks.h>
#include <vil/vil_image_view.h>
#include <vil/vil_save.h>
#include <vil/vil_math.h>
#endif

#ifdef TIMING_TEST
#include <boost/chrono.hpp>
#endif

#ifdef HAS_VXL

///adds a square to an image at \x,\y with side 2*\s
void add_square(vil_image_view<vxl_byte> &img, int x, int y, int s)
{
  for (int i = x - s; i <= x + s; i++)
  {
    for (int j = y - s; j <= y + s; j++)
    {
      int d = std::max<int>(abs(i - x), abs(j - y));
      if (img.in_range(i,j,0))
        img(i,j) = (vxl_byte)(((float)d/(float)s)*127.0f);
    }
  }
}

//Jitter diff find min difference
void jitter_diff_pair(const vil_image_view<vxl_byte> &img1,
                      const vil_image_view<vxl_byte> &img2,
                      vil_image_view<vxl_byte> &diff,
                      int jitter_delta)
{
  for (int i = 0; i < img1.ni(); i++)
  {
    for (int j = 0; j < img1.nj(); j++)
    {
      int mindiff = 255;
      for (int offi = i - jitter_delta; offi <= i + jitter_delta; offi++)
      {
        for (int offj = j - jitter_delta; offj <= j + jitter_delta; offj++)
        {
          if (img2.in_range(offi, offj, 0))
          {
            int a = img2(offi,offj);
            int b = img1(i,j);
            int diff = abs(a-b);
            if (diff < mindiff)
              mindiff = diff;
          }
        }
      }

      diff(i,j) = mindiff;
    }
  }
}

//jitter diff using intermediate val theorem
void jitter_diff_pair_min_max(const vil_image_view<vxl_byte> &img1,
                              const vil_image_view<vxl_byte> &img2,
                              vil_image_view<vxl_byte> &diff,
                              int jitter_delta)
{
  vil_image_view<vxl_byte> minv(img2.ni(), img2.nj());
  vil_image_view<vxl_byte> maxv(img2.ni(), img2.nj());
  minv.fill(255);
  maxv.fill(0);

  for (int i = 0; i < img2.ni(); i++)
  {
    for (int j = 0; j < img2.nj(); j++)
    {
      for (int offi = i - jitter_delta; offi <= i + jitter_delta; offi++)
      {
        for (int offj = j - jitter_delta; offj <= j + jitter_delta; offj++)
        {
          if (img2.in_range(offi, offj, 0))
          {
            vxl_byte val = img2(offi, offj);
            if (maxv(i,j) < val)
              maxv(i,j) = val;
            if (minv(i,j) > val)
              minv(i,j) = val;
          }
        }

        if (img1(i,j) < minv(i,j))
          diff(i,j) = minv(i,j) - img1(i,j);
        else if (img1(i,j) < maxv(i,j))
          diff(i,j) = 0;
        else
          diff(i,j) = img1(i,j) - maxv(i,j);
      }
    }
  }
}

//Implements: diff = | jit(| A - C |) + jit(| C - B |) - jit(| A - B |) |
//assumes all are same size
void jitter_diff_vxl(const vil_image_view<vxl_byte> &A,
                     const vil_image_view<vxl_byte> &B,
                     const vil_image_view<vxl_byte> &C,
                     vil_image_view<vxl_byte> &diff,
                     int jitter_delta)
{
  unsigned int ni = A.ni(), nj = A.nj();

  vil_image_view<vxl_byte> ac(ni, nj);
  vil_image_view<vxl_byte> cb(ni, nj);
  vil_image_view<vxl_byte> ab(ni, nj);

  jitter_diff_pair(A, C, ac, jitter_delta);
  jitter_diff_pair(C, B, cb, jitter_delta);
  jitter_diff_pair(A, B, ab, jitter_delta);

  vil_image_view<vxl_byte> temp;
  vil_math_image_sum(ac, cb, temp);
  vil_math_image_abs_difference(temp, ab, diff);
}



#endif

static void run_test(std::string const& test_name);

int
main(int argc, char* argv[])
{
  if (argc != 2)
  {
    TEST_ERROR("Expected one argument");

    return EXIT_FAILURE;
  }

  std::string const test_name = argv[1];

  try
  {
    run_test(test_name);
  }
  catch(const cl::Error &e)
  {
    std::cerr << "ERROR: " << e.what() << " (" << e.err() << " : "
             << viscl::print_cl_errstring(e.err()) << ")" << std::endl;
    return 1;
  }
  catch (std::exception const& e)
  {
    TEST_ERROR("Unexpected exception: " << e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static void test_jitter_diff_vxl();

void
run_test(std::string const& test_name)
{
  if (test_name == "jitter_diff_vxl")
  {
    test_jitter_diff_vxl();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}

void
test_jitter_diff_vxl()
{
#ifdef HAS_VXL
  const unsigned int width = 1000, height = 1000;
  const int jitter_delta = 1;

  vil_image_view<vxl_byte> A(width, height);
  vil_image_view<vxl_byte> B(width, height);
  vil_image_view<vxl_byte> C(width, height);

  A.fill(127);  B.fill(127);  C.fill(127);

  add_square(A, 100, 100, 50);
  add_square(B, 100, 100, 50);
  add_square(C, 100, 100, 50);

  add_square(A, 500, 500, 200);
  add_square(B, 501, 601, 200);
  add_square(C, 502, 702, 200);

  add_square(A, 100, 700, 50);
  add_square(B, 100, 700, 100);
  add_square(C, 100, 700, 150);

  vil_image_view<vxl_byte> diff_viscl;
  viscl::compute_jitter_difference(A, B, C, diff_viscl, jitter_delta);
  vil_image_view<vxl_byte> diff_vxl;
  jitter_diff_vxl(A, B, C, diff_vxl, jitter_delta);

#ifdef TIMING_TEST
  boost::chrono::system_clock::time_point start;
  boost::chrono::duration<double> sec;
  start = boost::chrono::system_clock::now();
  const int iter = 50;
  for (int i = 0; i < iter; i++)
    viscl::compute_jitter_difference(A, B, C, diff_viscl, jitter_delta);

  sec = boost::chrono::system_clock::now() - start;
  std::cout << "viscl took on average " << sec.count() / iter << "\n";
  start = boost::chrono::system_clock::now();

  for (int i = 0; i < iter; i++)
    jitter_diff_vxl(A, B, C, diff_vxl, jitter_delta);

  sec = boost::chrono::system_clock::now() - start;
  std::cout << "VXL took on avg"   << sec.count() / iter << "\n";
#endif

  vil_image_view<vxl_byte> diff;
  vil_math_image_abs_difference(diff_viscl, diff_vxl, diff);
  unsigned long diff_count = 0;
  for (unsigned i = 0; i < diff.ni(); i++)
  {
    for (unsigned int j = 0; j < diff.nj(); j++)
    {
      if (diff(i,j) != 0)
        diff_count++;
    }
  }
  if (diff_count > 0)
  {
    TEST_ERROR("GPU and CPU smoothing results differ in "
               << (100.0f*diff_count)/(diff.ni()*diff.nj()) << "% of pixels");
    vil_save(diff_viscl, "jdiff_viscl.png");
    vil_save(diff_vxl, "jdiff_vxl.png");
  }

#else
  TEST_ERROR("VXL support is not compiled in, this test is invalid");
#endif
}
