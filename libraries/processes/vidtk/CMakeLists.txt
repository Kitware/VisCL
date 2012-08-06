set(CXX_SRC_FILES
    track_descr_match_process.h             track_descr_match_process.cxx
)


add_library(viscl_vidtk_processes ${CXX_SRC_FILES})
target_link_libraries(viscl_vidtk_processes
  viscl_tasks viscl_vxl vil vgl vnl rrel vidtk_process_framework vidtk_kwklt)

install( TARGETS viscl_vidtk_processes EXPORT viscl_exports
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)
export( TARGETS viscl_vidtk_processes
  APPEND FILE "${viscl_export_file}"
)
