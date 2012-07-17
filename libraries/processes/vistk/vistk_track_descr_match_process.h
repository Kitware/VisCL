/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISTK_TRACK_DESCR_MATCH_PROCESS_H_
#define VISTK_TRACK_DESCR_MATCH_PROCESS_H_

#include "common/vistk_process.h"
#include "processes/vidtk/track_descr_match_process.h"

class vistk_track_descr_match_process : public viscl::vistk_process
{
public:

  vistk_track_descr_match_process(vistk::config_t const& conf);

protected:

  void _configure();
  void _step();
  void _init();

private:

  typedef track_descr_match_process viscl_proc_type;
  typedef boost::scoped_ptr<viscl_proc_type> viscl_proc_t;

  bool first;

  viscl_proc_t w_proc;
};



#endif
