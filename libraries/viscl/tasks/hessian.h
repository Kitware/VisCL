/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef HESSIAN_H_
#define HESSIAN_H_

#include <viscl/core/task.h>
#include <viscl/core/image.h>
#include <viscl/core/buffer.h>


namespace viscl
{

class hessian : public task
{
public:

  hessian();
  void smooth_and_detect(const image &img, image &kptmap, buffer &kpts, buffer &numkpts,
                         int max_kpts, float thresh, float sigma) const;
  void detect(const image &img, image &kptmap, buffer &kpts, buffer &numkpts,
              int max_kpts, float thresh, float scale) const;

  int num_kpts(const buffer &numkpts_b);

private:



  cl_kernel_t det_hessian, detect_extrema, init_kpt_map;
};

typedef boost::shared_ptr<hessian> hessian_t;

}

#endif
