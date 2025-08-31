cmake_minimum_required(VERSION 3.30)

project(minsql)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)
find_package(mysql-concpp REQUIRED)

include(${wxWidgets_USE_FILE})

if(WIN32)
	add_executable(${PROJECT_NAME} WIN32
		src/App.cpp
		src/DBPopup.cpp
		src/InitFrame.cpp
		src/MainFrame.cpp
		src/TPopup.cpp
		src/Util.cpp
	)
else()
	add_executable(${PROJECT_NAME}
		src/App.cpp
		src/DBPopup.cpp
		src/InitFrame.cpp
		src/MainFrame.cpp
		src/TPopup.cpp
		src/Util.cpp
	)
endif()

target_link_libraries(minsql ${wxWidgets_LIBRARIES})
target_link_libraries(minsql mysql::concpp-static)