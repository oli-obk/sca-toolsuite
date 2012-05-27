#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([sca-toolsuite], 0.10, johannes89 at mailueberfall dot de)
#AC_CONFIG_SRCDIR([src/add.h])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/core/Makefile
		 src/io/Makefile
		 src/algo/Makefile
		 src/math/Makefile
		 src/res/Makefile
		 src/rotor/Makefile
	 	 doc/Makefile
		 ])
#                src/tsa/Makefile
#                src/tsasim/Makefile
#                data/Makefile


AC_OUTPUT

#
CFLAGS=-O3
