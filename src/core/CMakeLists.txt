SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(create "${src_dir}/create.cpp"
	${general}
)

add_executable(all_equals "${src_dir}/all_equals.cpp"
	${general}
)

cp_script(diff2)

