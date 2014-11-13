#add_subdirectory(grids)
#add_subdirectory(input)
#add_subdirectory(ca)

SET(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${src_dir}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

option(VERBOSE_OUTPUT "dumps very verbose output to std::cerr" OFF)
option(NO_BOOST_GRAPH "disables boost::graph completely" OFF)
option(DEBUG_GRAPH "generate debug graph - takes much memory" OFF)
option(USE_LABELS "use node labels in debug graph" ON)
option(ADD_SO1 "add sort_out_1 nodes to debug graph" OFF)
option(ADD_SO2 "add sort_out_2 nodes to debug graph" OFF)

CONFIGURE_FILE(${src_dir}/brute_config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/brute_config.h)

add_executable(search
	"${src_dir}/greedy.cpp"
	"${src_dir}/base.cpp"
	"${src_dir}/base_io.cpp"
	"${src_dir}/results.cpp"
	"${src_dir}/brute_force.cpp"
	"${src_dir}/stats.cpp"
	"${src_dir}/search.cpp"
)

add_executable(eval "${src_dir}/eval.cpp" "${src_dir}/results.cpp")

target_link_libraries(search res)
target_link_libraries(eval res)


