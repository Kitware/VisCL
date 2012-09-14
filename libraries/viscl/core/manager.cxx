/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <viscl/core/manager.h>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace viscl
{


manager *manager::inst_ = 0;

//*****************************************************************************

manager *manager::inst()
{
  return inst_ ? inst_ : inst_ = new manager;
}

//*****************************************************************************

manager::manager()
{
  init_opencl();
}

//*****************************************************************************

void manager::init_opencl()
{
  try
  {
    // Get available platforms
    cl::Platform::get(&platforms_);

    /// \todo Add Windows support.
    char* def_platform = getenv("VISCL_DEFAULT_PLATFORM");

    size_t platform_id = DEFAULT_PLATFORM;
    if (def_platform)
    {
      try
      {
        platform_id = boost::lexical_cast<size_t>(def_platform);
      }
      catch (boost::bad_lexical_cast const& e)
      {
        std::string const reason = std::string("Failed to parse the platform: ") + e.what();

        throw std::runtime_error(reason);
      }
    }

    if (platforms_.size() <= platform_id)
    {
      std::stringstream reason;

      reason << "The platform ID requested (" << platform_id << ") is out of range: "
                "There are " << platforms_.size() << " available platforms.";

      throw std::runtime_error(reason.str());
    }

    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platforms_[platform_id])(),
        0
    };

    context_ =  cl::Context(CL_DEVICE_TYPE_GPU, cps);

    // Get a list of devices on this platform
    devices_ = context_.getInfo<CL_CONTEXT_DEVICES>();
  }
  catch(cl::Error &error)
  {
    std::cout << "Error: " << error.what() << " - " << print_cl_errstring(error.err()) << std::endl;
  }
}

//*****************************************************************************

cl_program_t manager::build_source(const char *source, int device) const
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
      std::string build_log = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices_[device]);
      std::cerr << build_log << std::endl;
    }
    throw error;
  }

  return program;
}

//*****************************************************************************

cl_queue_t manager::create_queue(int device)
{
  return boost::make_shared<cl::CommandQueue>(cl::CommandQueue(context_, devices_[device]));
}

//*****************************************************************************

image manager::create_image(const cl::ImageFormat &img_frmt, cl_mem_flags flags, size_t ni, size_t nj)
{
  return image(boost::make_shared<cl::Image2D>(cl::Image2D(context_,
                                                              flags,
                                                              img_frmt,
                                                              ni,
                                                              nj,
                                                              0,
                                                              0)));
}

//*****************************************************************************

void manager::report_device_specs(const cl::Device& dev,
                                     const std::string& prefix)
{
  try
  {
    std::cout << prefix << "Name                : "
              << dev.getInfo<CL_DEVICE_NAME>() << "\n";
    std::cout << prefix << "Type                : ";
    cl_device_type type = dev.getInfo<CL_DEVICE_TYPE>();
    if (type & CL_DEVICE_TYPE_CPU)
    {
      std::cout << "CPU ";
    }
    if (type & CL_DEVICE_TYPE_GPU)
    {
      std::cout << "GPU ";
    }
    if (type & CL_DEVICE_TYPE_ACCELERATOR)
    {
      std::cout << "Accelerator ";
    }
    std::cout << "\n";
    std::cout << prefix << "Vendor              : "
              << dev.getInfo<CL_DEVICE_VENDOR>() << "\n";
    std::cout << prefix << "Device Version      : "
              << dev.getInfo<CL_DEVICE_VERSION>() << "\n";
    std::cout << prefix << "Driver Version      : "
              << dev.getInfo<CL_DRIVER_VERSION>() << "\n";
    std::cout << prefix << "Max Clock Frequency : "
              << dev.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << " MHz\n";
    std::cout << prefix << "Max Compute Units   : "
              << dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
    std::cout << prefix << "Max Work Item Dims  : "
              << dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << "\n";
    std::cout << prefix << "Max Work Item Sizes : ";
    std::vector<size_t> work_sizes = dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    for (size_t i=0; i<work_sizes.size(); ++i)
    {
      std::cout << work_sizes[i];
      if (i < work_sizes.size()-1)
      {
        std::cout << ", ";
      }
    }
    std::cout << "\n";
    std::cout << prefix << "Max Work Group Size : "
              << dev.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << "\n";
    std::cout << prefix << "Global Memory       : "
              << dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()/1048576 << " Mb\n";
    std::cout << prefix << "Max Memory Alloc    : "
              << dev.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()/1048576 << " Mb\n";
    std::cout << prefix << "Max Image 2D Dims   : "
              << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>() << " x "
              << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>() << "\n";
    std::cout << prefix << "Max Image 3D Dims   : "
              << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>() << " x "
              << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>() << " x "
              << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>() << "\n";
    std::cout << prefix << "Out of Order Queue  : "
              << ((dev.getInfo<CL_DEVICE_QUEUE_PROPERTIES>() &
                  CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? "Yes" : "No") << "\n";


    std::string dev_ext = dev.getInfo<CL_DEVICE_EXTENSIONS>();
    std::istringstream extensions(dev_ext);
    std::string extension;
    std::cout << prefix << "Extensions          : ";
    bool first_ext = true;
    while (extensions >> extension)
    {
      if (first_ext)
      {
        first_ext = false;
      }
      else
      {
        std::cout << prefix << "                      ";
      }
      std::cout << extension << "\n";
    }
  }
  catch (cl::Error error)
  {
    std::cout << "Error: " << error.what() << " - "
             << print_cl_errstring(error.err()) << std::endl;
  }
}

//*****************************************************************************

void manager::report_opencl_specs()
{
  try
  {
    std::cout << "Found " << platforms_.size() << " Platforms" << std::endl;
    for (size_t i=0; i<platforms_.size(); ++i)
    {
      std::cout << "Platform[" << i << "]\n";
      std::cout << "  Name       : "
                << platforms_[i].getInfo<CL_PLATFORM_NAME>() << "\n";
      std::cout << "  Vendor     : "
                << platforms_[i].getInfo<CL_PLATFORM_VENDOR>() << "\n";
      std::cout << "  Version    : "
                << platforms_[i].getInfo<CL_PLATFORM_VERSION>() << "\n";
      std::cout << "  Profile    : "
                << platforms_[i].getInfo<CL_PLATFORM_PROFILE>() << "\n";
      std::string plat_ext = platforms_[i].getInfo<CL_PLATFORM_EXTENSIONS>();
      std::istringstream extensions(plat_ext);

      std::string extension;
      std::cout << "  Extensions : ";
      bool first_ext = true;
      while (extensions >> extension)
      {
        if (first_ext)
        {
          first_ext = false;
        }
        else
        {
          std::cout << "               ";
        }
        std::cout << extension << "\n";
      }

      std::vector<cl::Device> devices;
      platforms_[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
      std::cout << "  Found " << devices.size() << " Devices" << std::endl;
      for (size_t j=0; j<devices.size(); ++j)
      {
        std::cout << "\n  Device[" << j << "]\n";
        report_device_specs(devices[j], "    ");
      }
    }
  }
  catch (cl::Error error)
  {
    std::cout << "Error: " << error.what() << " - "
              << print_cl_errstring(error.err()) << std::endl;
  }
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

}
