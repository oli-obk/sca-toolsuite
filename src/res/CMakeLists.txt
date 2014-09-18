SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB lib_src ${src_dir}/*.cpp ${src_dir}/io/*.cpp)
file(GLOB lib_hdr ${src_dir}/*.h)

add_library(res SHARED ${lib_src} ${lib_hdr})


