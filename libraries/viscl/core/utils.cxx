/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "utils.h"

#include <boost/scoped_array.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace viscl
{

envvar_value_t viscl_getenv(char const* name)
{
  envvar_value_t value;

#if defined(_WIN32) || defined(_WIN64)
  DWORD sz = GetEnvironmentVariable(name, NULL, 0);

  if (sz)
  {
    typedef boost::scoped_array<char> raw_envvar_value_t;
    raw_envvar_value_t const envvalue(new char[sz]);

    sz = GetEnvironmentVariable(name, envvalue.get(), sz);

    value = envvalue.get();
  }

  if (!sz)
  {
    // Failed to read the environment variable.
    return envvar_value_t();
  }
#else
  char const* const envvalue = getenv(name);

  if (envvalue)
  {
    value = envvalue;
  }
#endif

  return value;
}

}
