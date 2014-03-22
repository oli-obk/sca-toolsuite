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

cp_script(diff2)

