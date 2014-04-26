SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(create "${src_dir}/create.cpp"
	${general} ${geometry}
)

add_executable(all_equals "${src_dir}/all_equals.cpp"
	${general} ${geometry}
)

cp_script(diff2)

