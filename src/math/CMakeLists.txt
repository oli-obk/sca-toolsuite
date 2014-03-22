SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(add "${src_dir}/add.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
)

add_executable(comb "${src_dir}/comb.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
)

add_executable(calc "${src_dir}/calc.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
	"${res_dir}/equation_solver.cpp"
	"${res_dir}/equation_solver.h"
)

add_executable(equation "${src_dir}/equation.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
	"${res_dir}/equation_solver.cpp"
	"${res_dir}/equation_solver.h"
)

cp_script(add2)
cp_script(sub2)
cp_script(coords)

