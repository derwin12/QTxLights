cmake_minimum_required(VERSION 3.21)
project(QTxLights VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/Qt/6.10.2/mingw_64" CACHE PATH "Qt6 prefix path")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(QTxLights WIN32
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/SequenceGrid.h
    src/SequenceGrid.cpp
    src/XsqFile.h
    src/XsqFile.cpp
)

target_link_libraries(QTxLights PRIVATE Qt6::Widgets)

# Enable /Zc:__cplusplus for MSVC to properly report C++17 support
if(MSVC)
    target_compile_options(QTxLights PRIVATE /Zc:__cplusplus)
endif()
