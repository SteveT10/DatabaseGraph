cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)

project(DatabaseGraph VERSION 1.0)

find_package(SQLite3 REQUIRED)
find_package(wxWidgets COMPONENTS net core base REQUIRED CONFIG)

if(wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif()

include_directories(${SQLite3_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp src/ChartCanvas.cpp src/ChartWindow.cpp src/ValueColumn.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLite3_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)