/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "cl_manager.h"

#include <vcl_iostream.h>
#include <vil/vil_copy.h>
#include <vcl_sstream.h>

cl_manager *cl_manager::inst_ = 0;

//*****************************************************************************

cl_manager *cl_manager::inst()
{
  return inst_ ? inst_ : inst_ = new cl_manager;
}

//*****************************************************************************

cl_manager::cl_manager()
{
  init_opencl();
  make_pixel_format_map();
}

//*****************************************************************************

void cl_manager::init_opencl()
{
  try
  {
    // Get available platforms
    cl::Platform::get(&platforms_);

    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platforms_[0])(),
        0
    };

    context_ =  cl::Context(CL_DEVICE_TYPE_GPU, cps);

    // Get a list of devices on this platform
    devices_ = context_.getInfo<CL_CONTEXT_DEVICES>();
  }
  catch(cl::Error &error)
  {
    vcl_cout << "Error: " << error.what() << " - " << print_cl_errstring(error.err()) << vcl_endl;
  }
}

//*****************************************************************************

cl_program_t cl_manager::build_source(const char *source, int device) const
{
  cl::Program::Sources src(1, std::make_pair(source, strlen(source)+1));

  // Make program of the source code in the context
  cl_program_t program = boost::make_shared<cl::Program>(cl::Program(context_, src));

  // Build program for these specific devices
  try {
    program->build(devices_);
  }
  catch(cl::Error error)  {
    if(error.err() == CL_BUILD_PROGRAM_FAILURE)
    {
      vcl_string build_log = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices_[device]);
      vcl_cerr << build_log << vcl_endl;
    }
    throw error;
  }

  return program;
}

//*****************************************************************************

cl_queue_t cl_manager::create_queue(int device)
{
  return boost::make_shared<cl::CommandQueue>(cl::CommandQueue(context_, devices_[device]));
}

//*****************************************************************************

//Does NOT support multiplane images or non-continuous memory
template<class T>
cl_image cl_manager::create_image(const vil_image_view<T> &img)
{
  if (!img.top_left_ptr())
  {
    vcl_cerr << "No image data!\n";
  }

  vil_pixel_format pf = img.pixel_format();
  vcl_map<vil_pixel_format, cl::ImageFormat>::iterator itr;
  if ((itr = pixel_format_map_.find(pf)) == pixel_format_map_.end())
    return cl_image();

  const cl::ImageFormat &img_fmt = itr->second;
  return cl_image(boost::make_shared<cl::Image2D>(cl::Image2D(context_,
                                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                  img_fmt,
                                                  img.ni(),
                                                  img.nj(),
                                                  0,
                                                  (T *)img.top_left_ptr())));
}

//*****************************************************************************

cl_image cl_manager::create_image(const cl::ImageFormat &img_frmt, cl_mem_flags flags, size_t ni, size_t nj)
{
  return cl_image(boost::make_shared<cl::Image2D>(cl::Image2D(context_,
                                                              flags,
                                                              img_frmt,
                                                              ni,
                                                              nj,
                                                              0,
                                                              0)));
}

//*****************************************************************************

void cl_manager::report_device_specs(const cl::Device& dev,
                                     const vcl_string& prefix)
{
  try
  {
    vcl_cout << prefix << "Name                : "
             << dev.getInfo<CL_DEVICE_NAME>() << "\n";
    vcl_cout << prefix << "Type                : ";
    cl_device_type type = dev.getInfo<CL_DEVICE_TYPE>();
    if (type & CL_DEVICE_TYPE_CPU)
    {
      vcl_cout << "CPU ";
    }
    if (type & CL_DEVICE_TYPE_GPU)
    {
      vcl_cout << "GPU ";
    }
    if (type & CL_DEVICE_TYPE_ACCELERATOR)
    {
      vcl_cout << "Accelerator ";
    }
    vcl_cout << "\n";
    vcl_cout << prefix << "Vendor              : "
             << dev.getInfo<CL_DEVICE_VENDOR>() << "\n";
    vcl_cout << prefix << "Device Version      : "
             << dev.getInfo<CL_DEVICE_VERSION>() << "\n";
    vcl_cout << prefix << "Driver Version      : "
             << dev.getInfo<CL_DRIVER_VERSION>() << "\n";
    vcl_cout << prefix << "Max Clock Frequency : "
             << dev.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz\n";
    vcl_cout << prefix << "Max Compute Units   : "
             << dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
    vcl_cout << prefix << "Max Work Item Dims  : "
             << dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << "\n";
    vcl_cout << prefix << "Max Work Item Sizes : ";
    vcl_vector<size_t> work_sizes = dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    for (size_t i=0; i<work_sizes.size(); ++i)
    {
      vcl_cout << work_sizes[i];
      if (i < work_sizes.size()-1)
      {
        vcl_cout << ", ";
      }
    }
    vcl_cout << "\n";
    vcl_cout << prefix << "Max Work Group Size : "
             << dev.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
    vcl_cout << prefix << "Global Memory       : "
             << dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()/1048576 << " Mb\n";
    vcl_cout << prefix << "Max Memory Alloc    : "
             << dev.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()/1048576 << " Mb\n";
    vcl_cout << prefix << "Max Image 2D Dims   : "
             << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>() << " x "
             << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>() << "\n";
    vcl_cout << prefix << "Max Image 3D Dims   : "
             << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>() << " x "
             << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>() << " x "
             << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>() << "\n";
    vcl_cout << prefix << "Out of Order Queue  : "
             << ((dev.getInfo<CL_DEVICE_QUEUE_PROPERTIES>() &
                  CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? "Yes" : "No") << "\n";


    vcl_string dev_ext = dev.getInfo<CL_DEVICE_EXTENSIONS>();
    vcl_istringstream extensions(dev_ext);
    vcl_string extension;
    vcl_cout << prefix << "Extensions          : ";
    bool first_ext = true;
    while (extensions >> extension)
    {
      if (first_ext)
      {
        first_ext = false;
      }
      else
      {
        vcl_cout << prefix << "                      ";
      }
      vcl_cout << extension << "\n";
    }
  }
  catch (cl::Error error)
  {
    vcl_cout << "Error: " << error.what() << " - "
             << print_cl_errstring(error.err()) << vcl_endl;
  }
}

//*****************************************************************************

void cl_manager::report_opencl_specs()
{
  try
  {
    vcl_cout << "Found " << platforms_.size() << " Platforms" << vcl_endl;
    for (size_t i=0; i<platforms_.size(); ++i)
    {
      vcl_cout << "Platform[" << i << "]\n";
      vcl_cout << "  Name       : "
               << platforms_[i].getInfo<CL_PLATFORM_NAME>() << "\n";
      vcl_cout << "  Vendor     : "
               << platforms_[i].getInfo<CL_PLATFORM_VENDOR>() << "\n";
      vcl_cout << "  Version    : "
               << platforms_[i].getInfo<CL_PLATFORM_VERSION>() << "\n";
      vcl_cout << "  Profile    : "
               << platforms_[i].getInfo<CL_PLATFORM_PROFILE>() << "\n";
      vcl_string plat_ext = platforms_[i].getInfo<CL_PLATFORM_EXTENSIONS>();
      vcl_istringstream extensions(plat_ext);

      vcl_string extension;
      vcl_cout << "  Extensions : ";
      bool first_ext = true;
      while (extensions >> extension)
      {
        if (first_ext)
        {
          first_ext = false;
        }
        else
        {
          vcl_cout << "               ";
        }
        vcl_cout << extension << "\n";
      }

      vcl_vector<cl::Device> devices;
      platforms_[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
      vcl_cout << "  Found " << devices.size() << " Devices" << vcl_endl;
      for (size_t j=0; j<devices.size(); ++j)
      {
        vcl_cout << "\n  Device[" << j << "]\n";
        report_device_specs(devices[j], "    ");
      }
    }
  }
  catch (cl::Error error)
  {
    vcl_cout << "Error: " << error.what() << " - "
             << print_cl_errstring(error.err()) << vcl_endl;
  }
}

//*****************************************************************************

//http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/cl_image_format.html
void cl_manager::make_pixel_format_map()
{
  pixel_format_map_[VIL_PIXEL_FORMAT_FLOAT] = cl::ImageFormat(CL_INTENSITY, CL_FLOAT);
  pixel_format_map_[VIL_PIXEL_FORMAT_BYTE] = cl::ImageFormat(CL_INTENSITY, CL_UNORM_INT8);
}

//*****************************************************************************

//Returns an error string explaining an error code
const char *print_cl_errstring(cl_int err)
{
    switch (err) {
        case CL_SUCCESS:                          return "Success";
        case CL_DEVICE_NOT_FOUND:                 return "Device not found";
        case CL_DEVICE_NOT_AVAILABLE:             return "Device not available";
        case CL_COMPILER_NOT_AVAILABLE:           return "Compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return "Memory object allocation failure";
        case CL_OUT_OF_RESOURCES:                 return "Out of resources";
        case CL_OUT_OF_HOST_MEMORY:               return "Out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:     return "Profiling information not available";
        case CL_MEM_COPY_OVERLAP:                 return "Memory copy overlap";
        case CL_IMAGE_FORMAT_MISMATCH:            return "Image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return "Image format not supported";
        case CL_BUILD_PROGRAM_FAILURE:            return "Program build failure";
        case CL_MAP_FAILURE:                      return "Map failure";
        case CL_INVALID_VALUE:                    return "Invalid value";
        case CL_INVALID_DEVICE_TYPE:              return "Invalid device type";
        case CL_INVALID_PLATFORM:                 return "Invalid platform";
        case CL_INVALID_DEVICE:                   return "Invalid device";
        case CL_INVALID_CONTEXT:                  return "Invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:         return "Invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:            return "Invalid command queue";
        case CL_INVALID_HOST_PTR:                 return "Invalid host pointer";
        case CL_INVALID_MEM_OBJECT:               return "Invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return "Invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:               return "Invalid image size";
        case CL_INVALID_SAMPLER:                  return "Invalid sampler";
        case CL_INVALID_BINARY:                   return "Invalid binary";
        case CL_INVALID_BUILD_OPTIONS:            return "Invalid build options";
        case CL_INVALID_PROGRAM:                  return "Invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:       return "Invalid program executable";
        case CL_INVALID_KERNEL_NAME:              return "Invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:        return "Invalid kernel definition";
        case CL_INVALID_KERNEL:                   return "Invalid kernel";
        case CL_INVALID_ARG_INDEX:                return "Invalid argument index";
        case CL_INVALID_ARG_VALUE:                return "Invalid argument value";
        case CL_INVALID_ARG_SIZE:                 return "Invalid argument size";
        case CL_INVALID_KERNEL_ARGS:              return "Invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:           return "Invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:          return "Invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:           return "Invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:            return "Invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:          return "Invalid event wait list";
        case CL_INVALID_EVENT:                    return "Invalid event";
        case CL_INVALID_OPERATION:                return "Invalid operation";
        case CL_INVALID_GL_OBJECT:                return "Invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:              return "Invalid buffer size";
        case CL_INVALID_MIP_LEVEL:                return "Invalid mip-map level";
        default:                                  return "Unknown";
    }
}

//*****************************************************************************

template cl_image cl_manager::create_image<float>(const vil_image_view<float> &);
template cl_image cl_manager::create_image<vxl_byte>(const vil_image_view<vxl_byte> &);
