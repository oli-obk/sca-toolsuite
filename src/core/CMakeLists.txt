#
# Compile
#

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${WARN_EXC} -std=c++11 -pipe")

SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(create "${src_dir}/create.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
)

add_executable(all_equals "${src_dir}/all_equals.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
)
