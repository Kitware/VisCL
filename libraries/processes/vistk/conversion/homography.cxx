/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "homography.h"

#include "timestamp.h"

image_to_image_homography_conversion::vistk_t
image_to_image_homography_conversion
::operator () (vidtk_t const& from) const
{
  vistk_t vistk_h;

  timestamp_conversion const ts_conv;

  vistk_h.set_transform(from.get_transform());
  vistk_h.set_source(ts_conv(from.get_source_reference()));
  vistk_h.set_destination(ts_conv(from.get_dest_reference()));

  return vistk_h;
}

image_to_image_homography_conversion::vidtk_t
image_to_image_homography_conversion
::operator () (vistk_t const& from) const
{
  vidtk_t vidtk_h;

  timestamp_conversion const ts_conv;

  vidtk_h.set_transform(from.transform());
  vidtk_h.set_source_reference(ts_conv(from.source()));
  vidtk_h.set_dest_reference(ts_conv(from.destination()));

  return vidtk_h;
}
