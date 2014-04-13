SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(rotor "${src_dir}/rotor.cpp" ${general})

cp_script(xrotor)

