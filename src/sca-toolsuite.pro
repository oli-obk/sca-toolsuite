######################################################################
# Automatically generated by qmake (2.01a) Sa. Jan 14 14:33:43 2012
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += . algo core io math res tsa
INCLUDEPATH += . res

# Input
HEADERS += res/io.h res/stack_algorithm.h \
	res/general.h \
	res/stack_algorithm_2.h \
	res/equation_solver.h \
	res/rotor_algorithm.h \
	res/image.h \
	res/asm_basic.h \
    gui_qt/MainWindow.h \
    gui_qt/DrawArea.h \
    gui_qt/StateMachine.h \
    gui_qt/MenuBar.h
SOURCES += algo/relax.cpp \
	core/create.cpp \
	io/avalanches_bin2human.cpp \
	io/field_to_seq.cpp \
	io/scat.cpp \
	io/seq_to_field.cpp \
	math/add.cpp \
	res/io.cpp \
	tsa/edges2tgf.cpp \
	tsa/graph2scctgf.cpp \
	tsa/grid2edges.cpp \
	algo/fix.cpp \
	algo/burning_test.cpp \
	tsasim/tsarun.cpp \
	core/all_equals.cpp \
	math/equation.cpp \
	res/equation_solver.cpp \
	math/comb.cpp \
	rotor/rotor.cpp \
	math/calc.cpp \
	algo/random_throw.cpp \
	io/convert.cpp \
	res/image.cpp \
	algo/id.cpp \
	io/to_tga.cpp \
	algo/super.cpp \
	gui_qt/main.cpp \
    gui_qt/MainWindow.cpp \
    gui_qt/DrawArea.cpp \
    gui_qt/StateMachine.cpp \
    gui_qt/MenuBar.cpp \
    math/filter.cpp \
    ca/ca.cpp \
    tsa/grid2tsagrid.cpp
OTHER_FILES += ../DOCUMENTATION \
	../INSTALL \
	../README \
	../FAQ \
	test.sh
