cmake_minimum_required(VERSION 3.5)
project(Application)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/mingw_64/lib/cmake" CACHE PATH "Qt install path")

find_package(Qt6 COMPONENTS Core Quick Network QuickControls2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    main.cpp
    Application.cpp
    WebClient.cpp
    TimerController.cpp
    USBHelper.cpp
)

set(HEADERS
    Application.h
    WebClient.h
    TimerController.h
    USBHelper.h
)

add_executable(Application ${SOURCES} ${HEADERS})
target_link_libraries(Application Qt6::Core Qt6::Quick Qt6::Network Qt6::QuickControls2)

if(WIN32)
    # Deploy Qt Quick Controls 2
    add_custom_command(TARGET Application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${Qt6_DIR}/../../../qml/QtQuick/Controls"
            $<TARGET_FILE_DIR:Application>/QtQuick/Controls
    )
endif()