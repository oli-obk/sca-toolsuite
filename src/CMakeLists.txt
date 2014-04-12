add_subdirectory(core)
add_subdirectory(algo)
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(ca)
add_subdirectory(rotor)
add_subdirectory(gui_qt)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/usr/CMakeLists.txt")
	add_subdirectory(usr)
endif()

cp_script(test.sh)

#add_subdirectory(tsa)
