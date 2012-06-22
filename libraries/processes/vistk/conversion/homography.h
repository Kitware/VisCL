/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_PROCESSES_PORTING_CONVERSION_HOMOGRAPHY_H
#define VIDTK_PROCESSES_PORTING_CONVERSION_HOMOGRAPHY_H

#include <utilities/homography.h>

#include <vistk/utilities/homography_types.h>

class image_to_image_homography_conversion
{
  public:
    typedef vistk::image_to_image_homography vistk_t;
    typedef vidtk::image_to_image_homography vidtk_t;

    vistk_t operator () (vidtk_t const& from) const;
    vidtk_t operator () (vistk_t const& from) const;
};

#endif // VIDTK_PROCESSES_PORTING_CONVERSION_HOMOGRAPHY_H
