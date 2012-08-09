/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_H
#define VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_H

#include <vistk/pipeline/process.h>
#include <vistk/pipeline/types.h>

#include <boost/scoped_ptr.hpp>

namespace vidtk
{

class process;

}

namespace viscl
{

class vistk_process
  : public vistk::process
{
  public:
    vistk_process(vistk::config_t const& conf);

    static constraint_t const constraint_vidtk;
  protected:
    virtual constraints_t _constraints() const;

    typedef vidtk::process* vidtk_process_t;

    void declare_vistk_configuration(vidtk_process_t vidtk_process);
    void configure_vidtk_process(vidtk_process_t vidtk_process);
  private:
    vistk::config::keys_t m_keys;
};

}

#endif // VIDTK_PROCESSES_PORTING_COMMON_VISTK_PROCESS_H
