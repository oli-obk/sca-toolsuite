#
# Compile
#

# set(WARN_EXC "-Wno-unused-local-typedefs -Wno-maybe-uninitialized")

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${WARN_EXC} -std=c++11 -pipe")

SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(edges2tgf "${src_dir}/edges2tgf.cpp" ${io_files})
add_executable(graph2scctgf "${src_dir}/graph2scctgf.cpp" ${io_files})
add_executable(grid2edges "${src_dir}/grid2edges.cpp" ${io_files})
add_executable(grid2tsagrid "${src_dir}/grid2tsagrid" ${io_files})


#toolsdir=../../install
#AM_CXXFLAGS = "-I../res"
#tools_PROGRAMS=scat field_to_seq seq_to_field avalanches_bin2human convert to_tga
#scat_SOURCES=scat.cpp ../res/io.cpp
# field_to_seq_SOURCES=field_to_seq.cpp ../res/io.cpp
# seq_to_field_SOURCES=seq_to_field.cpp ../res/io.cpp
# avalanches_bin2human_SOURCES=avalanches_bin2human.cpp ../res/io.cpp
# convert_SOURCES = convert.cpp ../res/io.cpp
# to_tga_SOURCES = to_tga.cpp ../res/io.cpp ../res/image.cpp

#EXTRA_DIST=to_ods








#set_target_properties(scat PROPERTIES COMPILE_FLAGS "-O3")
#set_target_properties(scat PROPERTIES LINK_FLAGS "-O3")
