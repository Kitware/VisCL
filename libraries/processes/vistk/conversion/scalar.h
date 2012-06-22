/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_PROCESSES_PORTING_CONVERSION_SCALAR_H
#define VIDTK_PROCESSES_PORTING_CONVERSION_SCALAR_H

#include <vector>

template <typename T>
class scalar_conversion
{
  public:
    typedef std::vector<T> vector_t;
    typedef T scalar_t;

    scalar_t
    operator () (vector_t const& from) const
    {
      if (!from.size())
      {
        return scalar_t();
      }

      scalar_t const scalar = from[0];

      return scalar;
    }

    vector_t
    operator () (scalar_t const& from) const
    {
      vector_t const vector;

      vector.push_back(from);

      return vector;
    }
};

#endif // VIDTK_PROCESSES_PORTING_CONVERSION_SCALAR_H
