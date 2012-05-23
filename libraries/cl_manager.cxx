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
    cl::Platform::get(&platforms);

    // Select the default platform and create a context using this platform and the GPU
    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)(platforms[0])(),
        0 
    };

    context =  cl::Context(CL_DEVICE_TYPE_GPU, cps);

    // Get a list of devices on this platform
    devices = context.getInfo<CL_CONTEXT_DEVICES>();
  }
  catch(cl::Error error)
  {
    vcl_cout << "Error: " << error.what() << " - " << print_cl_errstring(error.err()) << vcl_endl;
  }
}

//*****************************************************************************

cl::Program *cl_manager::build_source(const char *source, int device) const
{
  vcl_vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
  cl::Program::Sources source_(1, std::make_pair(source, strlen(source)+1));

  // Make program of the source code in the context
  cl::Program *program = new cl::Program(context, source_);

  // Build program for these specific devices
  try {
    program->build(devices);
  }
  catch(cl::Error error)  {
    if(error.err() == CL_BUILD_PROGRAM_FAILURE)
    {
      vcl_string build_log = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[device]);
      vcl_cerr << build_log << vcl_endl;
    }
    throw error;
  }

  return program;
}

//*****************************************************************************

cl::CommandQueue *cl_manager::create_queue(int device)
{
  return new cl::CommandQueue(context, devices[device]);
}

//*****************************************************************************

template<class T>
cl::Image2D *cl_manager::create_image(vil_image_view<T> &img, bool norm)
{
  vil_pixel_format pf = img.pixel_format();
  vcl_map<vil_pixel_format, cl::ImageFormat>::iterator itr;
  if (norm)
  {
    if ((itr = pixel_format_norm_map.find(pf)) == pixel_format_norm_map.end())
      return NULL;
  }
  else
  {
    if ((itr = pixel_format_map.find(pf)) == pixel_format_map.end())
      return NULL;
  }
  
  if (img.nplanes() == 3 || img.nplanes() == 4)
  {
    vil_image_view<T> img_to_write = vil_image_view<T>(img.ni(), img.nj(), 1, img.nplanes());
    vil_copy_reformat(img, img_to_write);
    img = img_to_write;
  }

  //Does not deal with non continuous memory atm
  return new cl::Image2D(context,
                         CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                         itr->second,
                         img.ni(),
                         img.nj(),
                         0,
                         (T *)img.top_left_ptr());
}

//*****************************************************************************

template<class T>
cl::Buffer *cl_manager::create_buffer(T *buf, size_t len)
{
  return new cl::Buffer(context, CL_MEM_READ_ONLY, len * sizeof(T), buf);
}

//*****************************************************************************

void cl_manager::report_system_specs(int device)
{
  vcl_cout << "***********Device Information***********\n";

  try {
    cl_ulong mem_size;
    clGetDeviceInfo(devices[device](), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
    vcl_cout << "Device global memory: " << mem_size/1048576 << " mb\n";

    const int bufsize = 1024;
    cl_char buf[bufsize];
    size_t len;
    clGetDeviceInfo(devices[device](), CL_DEVICE_EXTENSIONS, sizeof(cl_char)*bufsize, buf, &len);
    vcl_istringstream extensions(vcl_string((const char *)buf, len));
    vcl_string double_extension("cl_khr_fp64");
    bool has_double_extension = false;
  
    vcl_string extension;
    while (extensions >> extension)
    {
      if (extension == double_extension)
        has_double_extension = true;
    }

    vcl_cout << "Supports double extension? " << (has_double_extension ? "yes" : "no") << "\n";

    size_t width, height;
    clGetDeviceInfo(devices[device](), CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &width, NULL);
    clGetDeviceInfo(devices[device](), CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &height, NULL);
    vcl_cout << "Max image dimensions: " << width << "x" << height << "\n";
    
    cl_ulong max_alloc;
    clGetDeviceInfo(devices[device](), CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_alloc, NULL);
    vcl_cout << "Max memory allocation: " << max_alloc/1048576 << " mb\n";
  } 
  catch (cl::Error error) {
    vcl_cout << "Error: " << error.what() << " - " << print_cl_errstring(error.err()) << vcl_endl;
  }
}

//*****************************************************************************

void cl_manager::make_pixel_format_map()
{
  int offsets[3] = {0,
                    VIL_PIXEL_FORMAT_RGB_UINT_32 - VIL_PIXEL_FORMAT_UINT_32,
                    VIL_PIXEL_FORMAT_RGBA_UINT_32 - VIL_PIXEL_FORMAT_UINT_32};
  cl_channel_order order[3] = {CL_INTENSITY, CL_RGBx, CL_RGBA};
  for (unsigned int i = 0; i < 3; i++)
  {
    vcl_cout << offsets[i] << "\n";
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_UINT_32+offsets[i])] = cl::ImageFormat(order[i], CL_UNSIGNED_INT32);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_INT_32+offsets[i])] = cl::ImageFormat(order[i], CL_SIGNED_INT32);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_UINT_16+offsets[i])] = cl::ImageFormat(order[i], CL_UNSIGNED_INT16);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_INT_16+offsets[i])] = cl::ImageFormat(order[i], CL_SIGNED_INT16);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_BYTE+offsets[i])] = cl::ImageFormat(order[i], CL_UNSIGNED_INT8);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_SBYTE+offsets[i])] = cl::ImageFormat(order[i], CL_SIGNED_INT8);
    pixel_format_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_FLOAT+offsets[i])] = cl::ImageFormat(order[i], CL_FLOAT);
    //pixel_format_map[VIL_PIXEL_FORMAT_DOUBLE] needs extension
  }

  for (unsigned int i = 0; i < 3; i++)
  {
    pixel_format_norm_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_UINT_16+offsets[i])] = cl::ImageFormat(order[i], CL_UNORM_INT16);
    pixel_format_norm_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_INT_16+offsets[i])] = cl::ImageFormat(order[i], CL_SNORM_INT16);
    pixel_format_norm_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_BYTE+offsets[i])] = cl::ImageFormat(order[i], CL_UNORM_INT8);
    pixel_format_norm_map[(vil_pixel_format)(VIL_PIXEL_FORMAT_SBYTE+offsets[i])] = cl::ImageFormat(order[i], CL_SNORM_INT8);
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

//*****************************************************************************

template cl::Image2D *cl_manager::create_image<float>(vil_image_view<float> &, bool);
template cl::Image2D *cl_manager::create_image<vxl_byte>(vil_image_view<vxl_byte> &, bool);

template cl::Buffer *cl_manager::create_buffer<float>(float *, size_t);
template cl::Buffer *cl_manager::create_buffer<vxl_byte>(vxl_byte *, size_t);
