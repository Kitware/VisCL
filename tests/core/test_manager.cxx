/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include <vcl_iostream.h>

#include <test_common.h>

#include "cl_manager.h"

#include <cstdlib>
#include <exception>


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
  catch (std::exception const& e)
  {
    TEST_ERROR("Unexpected exception: " << e.what());

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static void test_report_opencl_specs();
static void test_create_image();

void
run_test(std::string const& test_name)
{
  if (test_name == "report_opencl_specs")
  {
    test_report_opencl_specs();
  }
  else if (test_name == "create_image")
  {
    test_create_image();
  }
  else
  {
    TEST_ERROR("Unknown test: " << test_name);
  }
}

void
test_report_opencl_specs()
{
  cl_manager::inst()->report_opencl_specs();
}

void
test_create_image()
{
  cl::ImageFormat img_frmt = cl::ImageFormat(CL_INTENSITY, CL_FLOAT);
  cl_manager::inst()->create_image(img_frmt, CL_MEM_READ_ONLY, 100, 100);
}



