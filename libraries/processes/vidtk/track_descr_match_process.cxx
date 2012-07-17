/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "track_descr_match_process.h"
#include "cl_task_registry.h"

#include <utilities/unchecked_return_value.h>

#include <boost/make_shared.hpp>


//NOTE: Use initialize() for all variable initializations.
track_descr_match_process::track_descr_match_process( vcl_string const& name )
  : process( name, "track_descr_match_process" ),
    cur_img_( NULL ),
    initialized_( false ),
    first_frame_set_(false)
{
  config_.add( "num_features", "1000" );
  config_.add( "search_range", "100" );
  tracker_ = NEW_VISCL_TASK(track_descr_match);
  tracks_last_ = new vcl_vector< vidtk::klt_track_ptr >;
  tracks_cur_ = new vcl_vector< vidtk::klt_track_ptr >;
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
    tracks_last_->resize(num_feat_);
    tracks_cur_->resize(num_feat_);
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
  active_tracks_.clear();
  new_tracks_.clear();

  if( !first_frame_set_ )
  {
    //For the first frame all tracks found are labeled as new
    vcl_vector<vnl_vector_fixed<double, 2> > kpts;
    tracker_->first_frame(cur_img_, kpts);

    new_tracks_.reserve(kpts.size());
    for (unsigned int i = 0; i < kpts.size(); i++)
    {
      vidtk::klt_track::point_ p;
      p.x = kpts[i][0];
      p.y = kpts[i][1];
      p.frame = 0;
      vidtk::klt_track_ptr tr = vidtk::klt_track::extend_track(p);
      (*tracks_cur_)[i] = tr;
      new_tracks_.push_back(tr);
    }

    first_frame_set_ = true;
  }
  else
  {
    //Tracker returns indices into the last frame for the current matches
    vcl_vector<vnl_vector_fixed<double, 2> > kpts;
    const vcl_vector<int> &indices = tracker_->track<vxl_byte>(cur_img_, kpts, (int)search_range_);

    assert(indices.size() == kpts.size());
    for (unsigned int i = 0; i < indices.size(); i++)
    {
      vidtk::klt_track::point_ p;
      p.x = kpts[i][0];
      p.y = kpts[i][1];
      p.frame = 0;

      //non match are labeled as -1 index
      vidtk::klt_track_ptr tr;
      if (indices[i] > -1)
      {
        tr = vidtk::klt_track::extend_track(p, (*tracks_last_)[indices[i]]);
        active_tracks_.push_back(tr);
      }
      else
      {
        tr = vidtk::klt_track::extend_track(p);
        new_tracks_.push_back(tr);
      }

      (*tracks_cur_)[i] = tr;
    }
  }

  //avoids copying the tracks yet again
  vcl_swap(tracks_cur_, tracks_last_);

  return true;
}


void
track_descr_match_process::set_image( vil_image_view< vxl_byte > const& img )
{
  cur_img_ = img;
}


vcl_vector< vidtk::klt_track_ptr > const& track_descr_match_process::active_tracks() const
{
  return active_tracks_;
}

vcl_vector<vidtk::klt_track_ptr> const& track_descr_match_process::created_tracks() const
{
  return new_tracks_;
}

