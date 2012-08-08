/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "timestamp.h"

timestamp_conversion::vistk_t
timestamp_conversion
::operator () (vidtk_t const& from) const
{
  if (from.has_time() && from.has_frame_number())
  {
    return vistk_t(from.time(), from.frame_number());
  }
  else if (from.has_time())
  {
    return vistk_t(from.time());
  }
  else if (from.has_frame_number())
  {
    return vistk_t(from.frame_number());
  }

  return vistk_t();
}

timestamp_conversion::vidtk_t
timestamp_conversion
::operator () (vistk_t const& from) const
{
  vidtk_t vidtk_ts;

  if (from.has_time())
  {
    vidtk_ts.set_time(from.time());
  }
  if (from.has_frame())
  {
    vidtk_ts.set_frame_number(from.frame());
  }

  return vidtk_ts;
}
