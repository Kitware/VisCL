add_executable(track track.cxx )
target_link_libraries(track ${OPENCL_LIBRARIES} vil vil_algo vul viscl)

add_executable(time_image_transfer time_image_transfer.cxx)
target_link_libraries(time_image_transfer ${OPENCL_LIBRARIES} vul viscl rt)
