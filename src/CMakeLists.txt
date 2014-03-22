add_subdirectory(core)
add_subdirectory(algo)
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(ca)
add_subdirectory(rotor)

if(EXISTS "usr/CMakeLists.txt")
 add_subdirectory(user)
endif()

cp_script(test.sh)

#add_subdirectory(tsa)
