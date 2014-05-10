SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(relax "${src_dir}/relax.cpp" ${general} ${geometry})
add_executable(fix "${src_dir}/fix.cpp" ${general} ${geometry})
add_executable(random_throw "${src_dir}/random_throw.cpp" ${general} ${geometry})
add_executable(burning_test "${src_dir}/burning_test.cpp" ${general} ${geometry})
add_executable(id "${src_dir}/id.cpp" ${general} ${geometry})
add_executable(super "${src_dir}/super.cpp" ${general} ${geometry})

cp_script(is_recurrent)
cp_script(l)
cp_script(L)
cp_script(s)
cp_script(S)
cp_script(throw)

