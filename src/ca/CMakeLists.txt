SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ca "${src_dir}/ca.cpp"
	${general}
	${eq_solver}
	${ca_basics}
	${ca}
)

add_executable(transf_by_grids "${src_dir}/transf_by_grids.cpp"
	${general}
	${geometry}
	${ca_basics}
)

cp_script(ca_file)

