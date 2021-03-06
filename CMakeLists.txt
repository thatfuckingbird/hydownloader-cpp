cmake_minimum_required(VERSION 3.12)

project(hydownloader-cpp LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_QT5 "Build against Qt5" OFF)

if(USE_QT5)
    find_package(Qt5 COMPONENTS Core Network Gui REQUIRED)
else()
    find_package(Qt6 COMPONENTS Core Network Gui REQUIRED)
endif()

add_library(hydownloader-cpp
    include/hydownloader-cpp/hydownloaderconnection.h
    src/hydownloaderconnection.cpp
    include/hydownloader-cpp/hydownloaderlogmodel.h
    src/hydownloaderlogmodel.cpp
    include/hydownloader-cpp/hydownloadersingleurlqueuemodel.h
    src/hydownloadersingleurlqueuemodel.cpp
    include/hydownloader-cpp/hydownloadersubscriptionmodel.h
    src/hydownloadersubscriptionmodel.cpp
    include/hydownloader-cpp/hydownloaderjsonobjectlistmodel.h
    src/hydownloaderjsonobjectlistmodel.cpp
    src/hydownloadersubscriptionchecksmodel.cpp
    include/hydownloader-cpp/hydownloadersubscriptionchecksmodel.h
    src/hydownloadermissedsubscriptionchecksmodel.cpp
    include/hydownloader-cpp/hydownloadermissedsubscriptionchecksmodel.h)

add_library(hydownloader-cpp::hydownloader-cpp ALIAS hydownloader-cpp)

target_include_directories(hydownloader-cpp PUBLIC include)

if(USE_QT5)
    target_link_libraries(hydownloader-cpp PRIVATE Qt5::Core Qt5::Network Qt5::Gui)
else()
    target_link_libraries(hydownloader-cpp PRIVATE Qt6::Core Qt6::Network Qt6::Gui)
endif()

install(TARGETS hydownloader-cpp LIBRARY DESTINATION lib)
