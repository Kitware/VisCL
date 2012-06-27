/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_descr_match_process.h"
#include "cl_task_registry.h"

#include <utilities/unchecked_return_value.h>


//NOTE: Use initialize() for all variable initializations.
track_descr_match_process::track_descr_match_process( vcl_string const& name )
  : process( name, "track_descr_match_process" ),
    cur_img_( NULL ),
    initialized_( false ),
    first_frame_set_(false)
{
  config_.add( "num_features", "500" );
  config_.add( "search_range", "100" );
  tracker_ = NEW_VISCL_TASK(track_descr_match);
}


track_descr_match_process::~track_descr_match_process()
{

}


vidtk::config_block track_descr_match_process::params() const
{
  return config_;
}


bool track_descr_match_process::set_params( vidtk::config_block const& blk )
{
  try
  {
    int num_feat_;
    blk.get( "num_features", num_feat_ );
    blk.get( "search_range", search_range_ );
    tracker_->set_max_kpts(num_feat_);
  }
  catch( vidtk::unchecked_return_value& )
  {
    // revert to previous values
    set_params( config_ );
    return false;
  }

  config_.update( blk );
  return true;
}


bool track_descr_match_process::initialize()
{
  if (!initialized_)
  {
    first_frame_set_ = false;
    initialized_ = true;
  }

  return true;
}

bool track_descr_match_process::reinitialize()
{
  this->initialized_ = false;
  return this->initialize();
}

bool track_descr_match_process::reset()
{
  return this->reinitialize();
}

bool track_descr_match_process::step()
{
  if( !first_frame_set_ )
  {
    tracker_->first_frame(cur_img_);
    first_frame_set_ = true;
  }
  else
  {
    tracks_ = tracker_->track<vxl_byte>(cur_img_, (int)search_range_);
  }

  // Mark that they have been used.
  cur_img_ = NULL;

  return true;
}


void
track_descr_match_process::set_image( vil_image_view< vxl_byte > const& img )
{
  cur_img_ = img;
}


vcl_vector< tdm_track > const&
track_descr_match_process::tracks() const
{
  return tracks_;
}

