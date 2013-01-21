/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef CL_UTILS_H_
#define CL_UTILS_H_

#include <boost/optional.hpp>

#include <string>

namespace viscl
{

typedef boost::optional<std::string> envvar_value_t;
envvar_value_t viscl_getenv(char const* name);

}

#endif
