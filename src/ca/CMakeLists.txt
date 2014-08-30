SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ca "${src_dir}/ca.cpp"
	${general}
	${geometry}
	${eq_solver}
	${ca_basics}
	${ca}
	${sim}
)

add_executable(dump "${src_dir}/dump.cpp"
	${general}
	${geometry}
	${eq_solver}
	${ca_basics}
	${ca}
	${sim}
)

add_executable(transf_by_grids "${src_dir}/transf_by_grids.cpp"
	${general}
	${geometry}
	${ca_basics}
)

add_executable(scene "${src_dir}/scene.cpp"
	${general}
	${geometry}
	${ca_basics}
)

cp_script(ca_file)

