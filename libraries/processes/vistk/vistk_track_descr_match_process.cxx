/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include "vistk_track_descr_match_process.h"
#include "common/vistk_process_macros.h"

#include <boost/make_shared.hpp>

//*****************************************************************************

vistk_track_descr_match_process::vistk_track_descr_match_process(vistk::config_t const& conf) 
  : viscl::vistk_process(conf),
    w_proc(new viscl_proc_type(name()))
{
  this->declare_vistk_configuration(w_proc.get());

  port_flags_t required, required_nodep;
  required.insert(flag_required);
  required_nodep.insert(flag_required);
  required_nodep.insert(flag_input_nodep);

  this->declare_input_port("image", boost::make_shared<port_info>("image/vil/byte/gray", required, "input image"));
  this->declare_input_port("reset", boost::make_shared<port_info>("bool", required_nodep, "Whether to reset the process or not."));
  
  this->declare_output_port("created_tracks", boost::make_shared<port_info>("tracks/klt/vidtk:created", required, "created tracks"));
  this->declare_output_port("active_tracks", boost::make_shared<port_info>("tracks/klt/vidtk:active", required, "active tracks"));
}

//*****************************************************************************

void vistk_track_descr_match_process::_configure()
{
  configure_vidtk_process(w_proc.get());
  vistk_process::_configure();
}

//*****************************************************************************

void vistk_track_descr_match_process::_init()
{
  first = true;
  vistk_process::_init();
}

//*****************************************************************************

void vistk_track_descr_match_process::_step()
{
  DEFINE_STEP_VARIABLES

  if (!first)
  {
    bool const reset = grab_from_port_as<bool>("reset");

    if (reset)
    {
      w_proc->reset();
    }
  }

  first = false;

  INPUT_BRIDGE(w_proc, image, vil_image_view<vxl_byte>);

  STEP_PROCESS(w_proc);

  IF_FAILED()
  {
    OUTPUT_COMPLETE(w_proc, active_tracks);
    OUTPUT_COMPLETE(w_proc, created_tracks);
    mark_process_as_complete();
  }
  else 
  {
    OUTPUT_BRIDGE(w_proc, active_tracks, vcl_vector< vidtk::klt_track_ptr >);
    OUTPUT_BRIDGE(w_proc, created_tracks, vcl_vector< vidtk::klt_track_ptr >);
  }

  vistk_process::_step();
}

//*****************************************************************************

