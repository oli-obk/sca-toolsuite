SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(scat "${src_dir}/scat.cpp" ${io_files})
add_executable(field_to_seq "${src_dir}/field_to_seq.cpp" ${io_files})
add_executable(seq_to_field "${src_dir}/seq_to_field.cpp" ${io_files})
add_executable(avalanches_bin2human
	"${src_dir}/avalanches_bin2human.cpp" ${io_files})
add_executable(convert "${src_dir}/convert.cpp" ${io_files})
add_executable(to_tga "${src_dir}/to_tga.cpp" ${io_files}
	"${res_dir}/image.cpp" "${res_dir}/image.h")

cp_script(to_image)
cp_script(to_ods)
cp_script(to_pdf)

