#
# General
#

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
project(sca-toolsuite)

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# help
# TODO: $myproject from project()
MESSAGE("This is the Makefile for ${MYPROJECT} (version ${VERSION}).")
MESSAGE("Options: (-D...=..., cmake -L for details)")

#
# Machine
#
IF(WIN32)
	SET(OS "win32")
ELSE(WIN32)
	SET(OS "linux")
ENDIF(WIN32)

SET(MY_HOST_X86 FALSE)
SET(MY_HOST_X86_64 FALSE)

IF(IS_X86)
	SET(MY_HOST_X86 TRUE)
	SET(MY_HOST_STR "x86")
ELSEIF(IS_X86_64)
	SET(MY_HOST_X86_64 TRUE)
	SET(MY_HOST_STR "x86_64")
ELSE(IS_X86)
	MESSAGE("Can't identify target host. Assuming 32 bit platform.")
	SET(MY_HOST_X86 TRUE)
	SET(MY_HOST_STR "x86")
ENDIF(IS_X86)

MESSAGE("-- Target host is ${OS}, ${MY_HOST_STR}")

#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/res/")
SET(src_base "${CMAKE_CURRENT_SOURCE_DIR}/src/")
SET(res_dir "${src_base}res/")

SET(io_files "${res_dir}/io.cpp" "${res_dir}/io.h" "${res_dir}/general.h" "${res_dir}/general.cpp")
SET(equation_solver "${res_dir}/equation_solver.cpp" "${res_dir}/equation_solver.h")
SET(ca "${res_dir}/ca.h")
SET(ca_basics "${res_dir}/ca_basics.h")


include_directories("${CMAKE_CURRENT_BINARY_DIR}src/") # for config.h
include_directories("${res_dir}")

#
# special headers
#

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
${CMAKE_CURRENT_BINARY_DIR}/src/config.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
${CMAKE_CURRENT_BINARY_DIR}/src/version.h)

# if(CUSTOM_INCLUDE_PATHS STREQUAL "")
# else()
# 	include_directories("${CUSTOM_INCLUDE_PATHS}")
# endif()
#link_directories("${ADD_LINK_DIRS}")

# INCLUDE (CheckIncludeFiles)



set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 COMPONENTS spirit)

#if(Boost_FOUND)
#	include_directories(${Boost_INCLUDE_DIRS})
#	add_executable(progname file1.cxx file2.cxx)
#	target_link_libraries(progname ${Boost_LIBRARIES})
#endif()

#
# Compile
#

set(WARN_EXC "-Wno-unused-local-typedefs -Wno-maybe-uninitialized")

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${WARN_EXC} -std=c++11 -pipe -g -O0")

add_subdirectory(src)

#target_link_libraries(calc ...)

# testing
enable_testing ()
#add_test (calc calc "7+3")
#TODO

#
# CPack Stuff
#

SET(MY_NAME_CAP "sca-toolsuite")
SET(MY_NAME_LOW "sca-toolsuite")
SET(MY_PACKAGE_FILE_NAME "${MY_NAME_LOW}-${VERSION}-${OS}-${MY_HOST_STR}")
SET(MY_SRC_PACKAGE_FILE_NAME "${MY_NAME_LOW}-${VERSION}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MY_NAME_CAP} - sca cellular automata toolsuite")
IF(WIN32)
SET(CPACK_PACKAGE_EXECUTABLES "${MY_NAME_LOW}.exe")
ELSE(WIN32)
SET(CPACK_PACKAGE_EXECUTABLES "${MY_NAME_LOW}")
SET(CPACK_STRIP_FILES "bin/${MY_NAME_LOW}") # TODO: bin?
ENDIF(WIN32)
SET(CPACK_PACKAGE_FILE_NAME ${MY_PACKAGE_FILE_NAME})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${MY_SRC_PACKAGE_FILE_NAME}")
SET(CPACK_PACKAGE_VENDOR "${MY_NAME_CAP} Developers")
SET(CPACK_PACKAGE_VERSION ${VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.txt")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${MY_SRC_PACKAGE_FILE_NAME}")
INCLUDE(CPack)
