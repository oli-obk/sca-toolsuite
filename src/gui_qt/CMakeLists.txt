# CMakeLists.txt for Qt GUI

set(gui_sources DrawArea.cpp main.cpp MainWindow.cpp MenuBar.cpp StateMachine.cpp CaSelector.cpp)
set(moc_headers DrawArea.h MainWindow.h MenuBar.h StateMachine.h CaSelector.h)
set(other_headers labeled_widget.h)

QT4_WRAP_CPP(gui_headers_moc ${moc_headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(gui ${gui_sources} ${gui_headers_moc} ${other_headers})
target_link_libraries(gui res ${QT_LIBRARIES})

