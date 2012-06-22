/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "homography_process.h"
#include <utilities/unchecked_return_value.h>
#include <utilities/log.h>
#include <vcl_cassert.h>
#include <vcl_fstream.h>
#include <vcl_algorithm.h>

#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vgl/algo/vgl_h_matrix_2d_optimize_lmq.h>

#include <vnl/vnl_math.h>

#include <rrel/rrel_homography2d_est.h>
#include <rrel/rrel_lms_obj.h>
#include <rrel/rrel_lts_obj.h>
#include <rrel/rrel_irls.h>
#include <rrel/rrel_ransac_obj.h>
#include <rrel/rrel_trunc_quad_obj.h>
#include <rrel/rrel_mlesac_obj.h>
#include <rrel/rrel_muset_obj.h>

//*****************************************************************************

//NOTE: Use initialize() for all variable initializations.
homography_process::homography_process( vcl_string const& name )
                    : process( name, "homography_process" ),
                      good_thresh_sqr_( 3*3 ),
                      current_ts_(NULL),
                      ransam_( NULL ),
                      initialized_( false )
{
  config_.add( "use_good_flag", "true" );
  config_.add( "lmq_refinement", "true" );
  config_.add_parameter( "good_threshold",
    "3",
    "Back-projection error (Euclidean distance)in pixels." );
}

//*****************************************************************************

homography_process::~homography_process()
{
  delete ransam_;
}

//*****************************************************************************

vidtk::config_block homography_process::params() const
{
  return config_;
}

//*****************************************************************************

bool homography_process::set_params( vidtk::config_block const& blk )
{
  try
  {
    double thr;
    blk.get( "good_threshold", thr );
    blk.get( "lmq_refinement", lmq_refinement_ );
    good_thresh_sqr_ = thr * thr;
  }
  catch( vidtk::unchecked_return_value& )
  {
    // reset to old values
    this->set_params( this->config_ );
    return false;
  }

  config_.update( blk );
  return true;
}

//*****************************************************************************

bool homography_process::initialize()
{
  if(!this->initialized_)
  {
    img_to_img0.set_identity(false);
    img0_to_img.set_identity(false);

    tracks_.clear();
    delete ransam_;
    ransam_ = new rrel_ran_sam_search( 42 );

    this->initialized_ = true;
    current_ts_ = (NULL);
    reference_ts_ = vidtk::timestamp();
  }

  return true;
}

//*****************************************************************************

bool homography_process::reset()
{
  this->initialized_ = false;
  return this->initialize();
}


//*****************************************************************************

bool homography_process::step()
{
  //First frame
  if(tracks_.empty())
  {
    img_to_img0.set_identity(true);

    if (current_ts_)
    {
      reference_ts_ = *current_ts_;

      img_to_img0.set_source_reference(*current_ts_)
        .set_dest_reference(reference_ts_)
        .set_new_reference(true);

      img0_to_img = img_to_img0.get_inverse();
    }

    return true;
  }

  vcl_vector< vnl_vector<double> > from_pts;
  vcl_vector< vnl_vector<double> > to_pts;

  vnl_vector<double> p(3);   p[2] = 1.0;
  for (unsigned int i = 0; i < tracks_.size(); i++)
  {
    const vnl_int_2 &pt = tracks_[i].pt_new;
    p[0] = tracks_[i].pt_new[0]; p[1] = tracks_[i].pt_new[1];
    from_pts.push_back( p );

    p[0] = tracks_[i].pt_prev[0]; p[1] = tracks_[i].pt_prev[1];
    to_pts.push_back( p );
  }

  // Step 2: estimate the homography using sampling.  This will allow
  // a good rejection of outliers.
  rrel_homography2d_est hg( from_pts, to_pts );

  rrel_trunc_quad_obj msac;
  ransam_->set_trace_level(0);

  bool result = ransam_->estimate( &hg, &msac );

  vidtk::image_to_image_homography cur_to_prev;
  cur_to_prev.set_identity(false);

  if ( ! result )
  {
    vcl_cout << name() << ": MSAC failed!!\n";
  }
  else
  {
    // Step 3: refine the estimate using weighted least squares.  This
    // will allow us to estimate a homography that does not exactly
    // fit 4 points, which will be a better estimate.  The ransam
    // estimate from step 2 would have gotten us close enough to the
    // correct solution for IRLS to work.
    rrel_irls irls;
    irls.initialize_params( ransam_->params() );
    bool result2 = irls.estimate( &hg, &msac );
    if( ! result2 )
    {
      // if the IRLS fails, fall back to the ransam estimate.
      vcl_cout << name() << ": IRLS failed\n";
      vnl_double_3x3 m;
      hg.params_to_homog( ransam_->params(), m.as_ref().non_const() );
      cur_to_prev.set_transform( vidtk::homography::transform_t(m) );
    }
    else
    {
      vnl_double_3x3 m;
      hg.params_to_homog( irls.params(), m.as_ref().non_const() );
      cur_to_prev.set_transform( vidtk::homography::transform_t(m) );
    }

    if (lmq_refinement_)
      lmq_refine_homography(cur_to_prev);
  }

  img_to_img0 = img_to_img0 * cur_to_prev;
  
  if( current_ts_ )
  {
    img_to_img0.set_source_reference(*current_ts_);

    if( !reference_ts_.is_valid() )
    {
      reference_ts_ = *current_ts_;
    }
  }

  img_to_img0.set_dest_reference( reference_ts_ );
  img_to_img0.set_valid(true);

  img0_to_img = img_to_img0.get_inverse();
  img0_to_img.set_new_reference (true) // dest ref is always new
             .set_valid(true);

  current_ts_ = NULL;

  return result;
}

//*****************************************************************************

void homography_process::set_timestamp( vidtk::timestamp const & ts )
{
  current_ts_ = &ts;
}

//*****************************************************************************

void homography_process::set_tracks( vcl_vector< tdm_track > const& trks )
{
  tracks_ = trks;
  good_flags_ = vcl_vector<bool>(false);
}

//*****************************************************************************

vgl_h_matrix_2d<double> const& homography_process::img_to_img0_homography() const
{
  return img_to_img0.get_transform();
}

//*****************************************************************************

vgl_h_matrix_2d<double> const& homography_process::img0_to_img_homography() const
{
  return img0_to_img.get_transform();
}

//*****************************************************************************

vidtk::image_to_image_homography const& homography_process::img_to_img0_i2i_homography() const
{
  return img_to_img0;
}

//*****************************************************************************

vidtk::image_to_image_homography const& homography_process::img0_to_img_i2i_homography() const
{
  return img0_to_img;
}

//*****************************************************************************

void homography_process::lmq_refine_homography(vidtk::image_to_image_homography &H)
{
  // Gather the "good" points.  At this point, we expect that the
  // outliers have been eliminated.  Refine the homography based on
  // geometric error.

  vcl_vector<vgl_homg_point_2d<double> > from_pts;
  vcl_vector<vgl_homg_point_2d<double> > to_pts;

  for(unsigned int i = 0; i < tracks_.size(); i++)
  {
    vgl_homg_point_2d<double> p_new( tracks_[i].pt_new[0], tracks_[i].pt_new[1] );
    vgl_homg_point_2d<double> q = H.get_transform() * p_new;
    vgl_homg_point_2d<double> p_prev( tracks_[i].pt_prev[0], tracks_[i].pt_prev[1] );
    double sqr_err = (q-p_prev).sqr_length();
    if( sqr_err <= good_thresh_sqr_) {
      from_pts.push_back( p_new );
      to_pts.push_back( p_prev );
    }
  }

  vgl_h_matrix_2d<double> refined_H;
  vgl_h_matrix_2d<double> initial_H( H.get_transform() );
  vgl_h_matrix_2d_optimize_lmq opt( initial_H );
  opt.set_verbose( false );
  if( opt.optimize( from_pts, to_pts, refined_H ) )
    H.set_transform(refined_H.get_matrix());
}


//*****************************************************************************
