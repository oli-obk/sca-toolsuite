#
# Compile
#

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${WARN_EXC} -std=c++11 -pipe")

SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(relax "${src_dir}/relax.cpp" ${io_files})
add_executable(fix "${src_dir}/fix.cpp" ${io_files})
add_executable(random_throw "${src_dir}/random_throw.cpp" ${io_files})
add_executable(burning_test "${src_dir}/burning_test.cpp" ${io_files})
add_executable(id "${src_dir}/id.cpp" ${io_files})
add_executable(super "${src_dir}/super.cpp" ${io_files})