find_package(Qt4 REQUIRED)

set(gui_sources DrawArea.cpp main.cpp MainWindow.cpp MenuBar.cpp StateMachine.cpp
	../res/io.cpp ../res/image.cpp ../res/general.cpp)
set(gui_headers DrawArea.h MainWindow.h MenuBar.h StateMachine.h)
set(other_headers ../res/io.h ../res/image.h ../res/general.h)

QT4_WRAP_CPP(gui_headers_moc ${gui_headers})

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_executable(gui ${gui_sources} ${gui_headers_moc} ${other_headers})
target_link_libraries(gui ${QT_LIBRARIES})
