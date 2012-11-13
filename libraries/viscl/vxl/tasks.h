/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VISCL_VXL_TASKS_H_
#define VISCL_VXL_TASKS_H_

#include <viscl/vxl/transfer.h>
#include <vil/vil_image_view.h>
#include <vgl/algo/vgl_h_matrix_2d.h>
#include <vnl/vnl_inverse.h>
#include <vgl/vgl_box_2d.h>
#include <vgl/vgl_intersection.h>
#include <vcl_vector.h>

#include <viscl/core/image.h>
#include <viscl/core/manager.h>
#include <viscl/tasks/hessian.h>
#include <viscl/tasks/BRIEF.h>
#include <viscl/tasks/gaussian_smooth.h>
#include <viscl/tasks/track_descr_match.h>
#include <viscl/tasks/warp_image.h>
#include <viscl/core/matrix.h>

#include <viscl/vxl/conversion.h>

namespace viscl
{

template <class T>
void cl_hessian_detect(const vil_image_view<T> &img, int max_kpts, float thresh,
                       float sigma, vcl_vector<cl_int2> &kpts)
{
  image img_cl = upload_image(img);
  hessian_t hes = NEW_VISCL_TASK(viscl::hessian);
  image kptmap;
  buffer numkpts_b, kpts_b;
  hes->smooth_and_detect(img_cl, kptmap, kpts_b, numkpts_b, max_kpts, thresh, sigma);

  cl_queue_t queue = hes->get_queue();
  int buf[1];
  queue->enqueueReadBuffer(*numkpts_b().get(), CL_TRUE, 0, numkpts_b.mem_size(), buf);
  int numkpts = buf[0];

  kpts.resize(numkpts);
  queue->enqueueReadBuffer(*kpts_b().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);
}


//*****************************************************************************

template <class T>
void cl_gaussian_smooth(const vil_image_view<T> &img, vil_image_view<T> &output,
                        float sigma, int kernel_radius)
{
  image img_cl = upload_image(img);
  gaussian_smooth_t gs = NEW_VISCL_TASK(viscl::gaussian_smooth);
  image result = gs->smooth( img_cl, sigma, kernel_radius);
  download_image(img_cl, output, gs->get_queue());
}

//*****************************************************************************

template<class pixtype>
void track_descr_first_frame(const vil_image_view<pixtype> &img,
                             vcl_vector<cl_int2> &kpts,
                             track_descr_match_t& tdm)
{
  image img_cl = upload_image(img);
  tdm->first_frame(img_cl);
  const buffer& kpts1 = tdm->last_keypoints();

  int numkpts = tdm->last_num_keypoints();
  kpts.clear();
  kpts.resize(numkpts);
  cl_queue_t queue = tdm->get_queue();
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);
}

//*****************************************************************************

template<class pixtype>
vcl_vector<int> track_descr_track(const vil_image_view<pixtype> &img,
                                  vcl_vector<cl_int2> &kpts,
                                  track_descr_match_t& tdm)
{
  image img_cl = upload_image(img);
  buffer tracks_b = tdm->track(img_cl);
  const buffer& kpts1 = tdm->last_keypoints();
  int numkpts = tdm->last_num_keypoints();

  vcl_vector<int> tracks;
  tracks.resize(numkpts);
  cl_queue_t queue = tdm->get_queue();
  queue->enqueueReadBuffer(*tracks_b(), CL_TRUE, 0, tracks_b.mem_size(), &tracks[0]);

  kpts.clear();
  kpts.resize(numkpts);
  queue->enqueueReadBuffer(*kpts1().get(), CL_TRUE, 0, sizeof(cl_int2)*numkpts, &kpts[0]);

  return tracks;
}

//*****************************************************************************

template<class T, int R>
void compute_brief_descriptors(const vil_image_view<T> &img,
                               const vcl_vector<cl_int2> &kpts,
                               vcl_vector<cl_int4> &descriptors,
                               float sigma)
{
  typename brief<R>::type brf = NEW_VISCL_TASK(viscl::brief<R>);
  gaussian_smooth_t gs = NEW_VISCL_TASK(viscl::gaussian_smooth);
  cl_queue_t queue = brf->get_queue();

  image img_cl = upload_image(img);
  image smoothed_cl = gs->smooth(img_cl, sigma, 2);
  buffer kpts_cl = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  buffer descriptors_cl;
  brf->compute_descriptors(smoothed_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}

//*****************************************************************************

template<class T, int R>
void compute_brief_descriptors(const vil_image_view<T> &img,
                               const vcl_vector<cl_int2> &kpts,
                               vcl_vector<cl_int4> &descriptors)
{
  typename brief<R>::type brf = NEW_VISCL_TASK(viscl::brief<R>);
  cl_queue_t queue = brf->get_queue();

  image img_cl = upload_image(img);
  buffer kpts_cl = manager::inst()->create_buffer<cl_int2>(CL_MEM_READ_ONLY, kpts.size());
  queue->enqueueWriteBuffer(*kpts_cl().get(), CL_TRUE, 0, kpts_cl.mem_size(), &kpts[0]);

  buffer descriptors_cl;
  brf->compute_descriptors(img_cl, kpts_cl, kpts.size(), descriptors_cl);
  descriptors.resize(kpts.size());
  queue->enqueueReadBuffer(*descriptors_cl().get(), CL_TRUE, 0, descriptors_cl.mem_size(), &descriptors[0]);
}

//*****************************************************************************

template <class pixType, class T>
bool warp_image_vxl(const vil_image_view<pixType> &src, vil_image_view<pixType> &dest, const vgl_h_matrix_2d<T> &H)
{
  warp_image_t warper = NEW_VISCL_TASK(viscl::warp_image);
  image dest_cl;
  size_t dni = dest.ni(), dnj = dest.nj();
  if (dni == 0 || dnj == 0)
  {
    dni = src.ni();
    dnj = src.nj();
  }

  if (!warp_image_vxl(src, dest_cl, dni, dnj, H, warper))
    return false;

  download_image(dest_cl, dest, warper->get_queue());

  return true;
}

//*****************************************************************************

template <class pixType, class T>
bool warp_image_vxl(const vil_image_view<pixType> &src, image &dest_cl, const size_t dni, const size_t dnj,
                    const vgl_h_matrix_2d<T> &H, const warp_image_t &warper)
{
  // Retrieve destination and source image properties
  unsigned int const sni = src.ni();
  unsigned int const snj = src.nj();

  typedef vgl_homg_point_2d<T> homog_type;
  typedef vgl_point_2d<T> point_type;
  typedef vgl_box_2d<T> box_type;

  // First, figure out the bounding box of src projected into dest.
  // There may be considerable computation saving for the cases when
  // the output image is much larger that the projected source image,
  // which will often occur during mosaicing.
  vgl_h_matrix_2d<T> const& src_to_dest_homography(vnl_inverse( H.get_matrix() ) );

  box_type src_on_dest_bounding_box;

  homog_type cnrs[4] = { homog_type( 0, 0 ),
                         homog_type( sni - 1, 0 ),
                         homog_type( sni - 1, snj - 1 ),
                         homog_type( 0, snj - 1 ) };

  for( unsigned i = 0; i < 4; ++i )
  {
    // Shift the point to destination image pixel index coordinates
    point_type p = src_to_dest_homography * cnrs[i];
    src_on_dest_bounding_box.add( p );
  }

  // Calculate intersection with destination pixels we are looking to fill
  box_type dest_boundaries( 0, dni - 1, 0, dnj - 1 );
  box_type intersection = vgl_intersection( src_on_dest_bounding_box, dest_boundaries );

  // Determine if this is a special case (source and destination images
  // overlap at only a single exact point). We can handle this case better
  // later.
  bool point_intercept = false;
  if( intersection.min_x() == intersection.max_x() &&
      intersection.min_y() == intersection.max_y() )
  {
    point_intercept = true;
  }

  // Exit on invalid case, or else semi-optimized warp will fail. This
  // condition should only occur if we exceed the image bounds given the
  // above computation (ie one image maps to a region completely outside
  // the other, or there is less than a 1 pixel overlap)
  if( intersection.width() == 0 && intersection.height() == 0 && !point_intercept)
  {
    return false;
  }

  // Extract start and end, row/col scanning ranges [start..end-1]
  const int start_j = static_cast<int>(std::floor(intersection.min_y()));
  const int start_i = static_cast<int>(std::floor(intersection.min_x()));
  const int end_j = static_cast<int>(std::floor(intersection.max_y()+1));
  const int end_i = static_cast<int>(std::floor(intersection.max_x()+1));

  image src_cl = upload_image(src);
  dest_cl = manager::inst()->create_image(src_cl.format(), CL_MEM_READ_WRITE, dni, dnj);
  viscl::matrix3x3 H_cl = vnl_matrix_fixed_3x3_to_viscl_homography(H.get_matrix());

  warper->warp(src_cl, dest_cl, H_cl, start_i, start_j, end_i, end_j);

  return true;
}


} // end namespace viscl

#endif // VISCL_VXL_TASKS_H_
