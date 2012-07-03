set(CXX_SRC_FILES
    track_descr_match_process.h             track_descr_match_process.cxx
)


add_library(processes ${CXX_SRC_FILES})
target_link_libraries(processes ${OPENCL_LIBRARIES} vil vgl vnl viscl vidtk_process_framework vidtk_kwklt rrel vistk_pipeline)

