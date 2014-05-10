SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(ca "${src_dir}/ca.cpp"
	${general}
	${geometry}
	${eq_solver}
	${ca_basics}
	${ca}
)

add_executable(transf_by_grids "${src_dir}/transf_by_grids.cpp"
	${general}
	${geometry}
	${ca_basics}
)

#call_test "Testing ca/ca (1)" 1 "core/create 20 20 0 | ca/ca 'v+2' end 4 | core/all_equals 8"
#call_test "Testing ca/ca (2)" 1 "core/create 20 20 4 | ca/ca 'v+(-4*(v>=4))+(a[-1,0]>=4)+(a[0,-1]>=4)+(a[1,0]>=4)+(a[0,1]>=4)' | core/diff2 'core/create 20 20 4 | algo/S'"
add_test ("ca (1)" core/create 20 20 0 | ca/ca 'v+2' end 4 | core/all_equals 8)
add_test ("ca (1)" core/create 20 20 0 | ca/ca 'v+2' end 4 | core/all_equals 9)

cp_script(ca_file)

