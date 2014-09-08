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
	gui_qt/MenuBar.h \
	res/random.h \
	res/ca_basics.h \
	res/ca.h \
	res/graph_io.h \
	res/geometry.h \
	res/eqs_internal.h \
	res/eqs_functions.h \
    gui_qt/CaSelector.h \
    gui_qt/labeled_widget.h \
    res/grid.h \
    res/patch.h \
    res/print.h \
    res/simulate.h \
    res/ca_convert.h \
    res/ca_table.h \
    res/bitgrid.h \
    res/ca_eqs.h \
    res/traits.h
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
	tsa/grid2tsagrid.cpp \
	ca/transf_by_grids.cpp \
	res/general.cpp \
	res/graph_io.cpp \
	res/geometry.cpp \
	res/eqs_functions.cpp \
	test/test.cpp \
	gui_qt/CaSelector.cpp \
	img/transform.cpp \
    ca/dump.cpp \
    res/simulate.cpp \
    res/ca.cpp \
    ca/scene.cpp
OTHER_FILES += ../DOCUMENTATION \
	../INSTALL.txt \
	../README.txt \
	../FAQ.txt \
	../BENCHMARKS.txt \
	../CMakeLists.txt \
	test.sh \
	CMakeLists.txt \
	core/CMakeLists.txt \
	algo/CMakeLists.txt \
	io/CMakeLists.txt \
	math/CMakeLists.txt \
	ca/CMakeLists.txt \
	rotor/CMakeLists.txt \
	gui_qt/CMakeLists.txt \
	test/CMakeLists.txt \
	img/CMakeLists.txt \
    res/CMakeLists.txt
