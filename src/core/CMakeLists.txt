SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(create "${src_dir}/create.cpp"
	${io_files}
)

add_executable(all_equals "${src_dir}/all_equals.cpp"
	${io_files}
)

cp_script(diff2)

