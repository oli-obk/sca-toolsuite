#
# Compile
#

# set(WARN_EXC "-Wno-unused-local-typedefs -Wno-maybe-uninitialized")

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${WARN_EXC} -std=c++11 -pipe")

SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ca "${src_dir}/ca.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
	"${res_dir}/equation_solver.cpp"
	"${res_dir}/equation_solver.h"
)

add_executable(transf_by_grids "${src_dir}/transf_by_grids.cpp"
	"${res_dir}/io.cpp"
	"${res_dir}/io.h"
	"${res_dir}/general.h"
)

#add_SOURCES=add.cpp ../res/io.cpp
#equation_SOURCES=equation.cpp ../res/io.cpp ../res/equation_solver.cpp
#comb_SOURCES=comb.cpp ../res/io.cpp
#calc_SOURCES=calc.cpp ../res/io.cpp ../res/equation_solver.cpp

#calc_SOURCES=calc.cpp ../res/io.cpp ../res/equation_solver.cpp 








#set_target_properties(scat PROPERTIES COMPILE_FLAGS "-O3")
#set_target_properties(scat PROPERTIES LINK_FLAGS "-O3")
