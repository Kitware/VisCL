/*ckwg +5
 * Copyright 2011-2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_TEST_TEST_COMMON_H
#define VISCL_TEST_TEST_COMMON_H

#include <iostream>

#define TEST_ERROR(msg)                         \
  do                                            \
  {                                             \
    std::cerr << "Error: " << msg << std::endl; \
  } while (false)

#define EXPECT_EXCEPTION(ex, code, action)  \
  do                                        \
  {                                         \
    bool got_exception = false;             \
                                            \
    try                                     \
    {                                       \
      code;                                 \
    }                                       \
    catch (ex const& e)                     \
    {                                       \
      got_exception = true;                 \
                                            \
      (void)e.what();                       \
    }                                       \
    catch (std::exception const& e)         \
    {                                       \
      TEST_ERROR("Unexpected exception: "   \
                 << e.what());              \
                                            \
      got_exception = true;                 \
    }                                       \
    catch (...)                             \
    {                                       \
      TEST_ERROR("Non-standard exception"); \
                                            \
      got_exception = true;                 \
    }                                       \
                                            \
    if (!got_exception)                     \
    {                                       \
      TEST_ERROR("Did not get "             \
                 "expected exception when " \
                 << action);                \
    }                                       \
  } while (false)

/// Make a simple checkerboard pattern image for use in tests.
/// Fills a pre-allocated buffer \a buffer of size \a width by \a height.
/// Each block of the checkerboard is \a blocksize by \a blocksize.
void make_checkerboard_image(const unsigned width,
                             const unsigned height,
                             unsigned char* const buffer,
                             const unsigned blocksize = 8)
{
  for (unsigned j=0; j<height; ++j)
  {
    for (unsigned i=0; i<width; ++i)
    {
      // index into the image buffer
      const unsigned index = j * width + i;
      // block cell coordinates
      const unsigned bi = i / blocksize;
      const unsigned bj = j / blocksize;
      // checkerboard boolean value (true for white, false for black)
      const bool val = (bi % 2 + bj % 2) % 2;
      // scale bool to byte image range
      buffer[index] = static_cast<unsigned char>(val * 255);
    }
  }
}

#endif // VISCL_TEST_TEST_COMMON_H
