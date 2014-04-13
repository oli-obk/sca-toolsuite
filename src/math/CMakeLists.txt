SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(add "${src_dir}/add.cpp"
	${general}
)

add_executable(comb "${src_dir}/comb.cpp"
	${general}
)

add_executable(calc "${src_dir}/calc.cpp"
	${general}
	${eq_solver}
)

add_executable(equation "${src_dir}/equation.cpp"
	${general}
	${eq_solver}
)

cp_script(add2)
cp_script(sub2)
cp_script(coords)

