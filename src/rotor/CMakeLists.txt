SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(rotor "${src_dir}/rotor.cpp" ${io_files})

cp_script(xrotor)

