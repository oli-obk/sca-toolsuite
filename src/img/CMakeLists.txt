SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

if(BUILD_IMG_MAGICK)
	add_definitions(-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
	include_directories(${ImageMagick_INCLUDE_DIRS})
	add_executable(transform "${src_dir}/transform.cpp" ${general} ${geometry} ${eq_solver} ${ca})
	target_link_libraries(transform ${ImageMagick_LIBRARIES} -ljpeg -lpng)
endif(BUILD_IMG_MAGICK)

#cp_script(to_image)

