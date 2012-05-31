#ifndef CL_UTIL_H_
#define CL_UTIL_H_

#include "cl_header.h"
#include "cl_image.h"


template<class T>
void save_cl_image(const cl_queue_t &queue, const cl_image &img, const char *filename);
template<>
void save_cl_image<unsigned char>(const cl_queue_t &queue, const cl_image &img, const char *filename);

#endif
