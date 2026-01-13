cmake_minimum_required(VERSION 3.16)

project(MyScannerApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport WinExtras)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/dtwain/include)

link_directories(${CMAKE_SOURCE_DIR}/3rdparty/dtwain/lib)

add_executable(MyScannerApp
    src/main.cpp
    src/MainWindow/MainWindow.cpp
    src/MainWindow/MainWindow.h
    src/ScanManager/ScanManager.cpp
    src/ScanManager/ScanManager.h
)

target_link_libraries(MyScannerApp 
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets 
        Qt5::PrintSupport
        Qt5::WinExtras
        ${dtwain32})

set_target_properties(MyScannerApp PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MyScannerApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    add_custom_command(TARGET MyScannerApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/3rdparty/dtwain/lib/dtwain32.dll"
        "$<TARGET_FILE_DIR:MyScannerApp>"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/3rdparty/dtwain/lib/dtwain32.ini"
        "$<TARGET_FILE_DIR:MyScannerApp>"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/3rdparty/dtwain/lib/twaininfo.txt"
        "$<TARGET_FILE_DIR:MyScannerApp>"
    )
endif()