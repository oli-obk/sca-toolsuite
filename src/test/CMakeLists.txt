SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(tests "${src_dir}/test.cpp"
	${general}
	${geometry}
	${ca_basics}
	${ca}
	${eq_solver}
)

