# CMakeLists.txt for Qt GUI

set(gui_sources DrawArea.cpp main.cpp MainWindow.cpp MenuBar.cpp StateMachine.cpp CaSelector.cpp
	../res/io.cpp ../res/image.cpp ../res/ca.cpp ../res/general.cpp ../res/geometry.cpp
	../res/equation_solver.cpp ../res/eqs_functions.cpp)
set(gui_headers DrawArea.h MainWindow.h MenuBar.h StateMachine.h CaSelector.h labeled_widget.h)
set(other_headers ../res/io.h ../res/image.h ../res/general.h ../res/geometry.h ../res/ca.h ../res/ca_basics.h
	../res/equation_solver.h ../res/eqs_internal.h ../res/eqs_functions.h)

QT4_WRAP_CPP(gui_headers_moc ${gui_headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(gui ${gui_sources} ${gui_headers_moc} ${other_headers})
target_link_libraries(gui ${QT_LIBRARIES})

