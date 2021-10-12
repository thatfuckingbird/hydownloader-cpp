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
    src/hydownloaderconnection.h
    src/hydownloaderconnection.cpp
    src/hydownloaderlogmodel.h
    src/hydownloaderlogmodel.cpp
    src/hydownloadersingleurlqueuemodel.h
    src/hydownloadersingleurlqueuemodel.cpp
    src/hydownloadersubscriptionmodel.h
    src/hydownloadersubscriptionmodel.cpp
    src/hydownloaderjsonobjectlistmodel.h
    src/hydownloaderjsonobjectlistmodel.cpp
    src/hydownloadersubscriptionchecksmodel.cpp
    src/hydownloadersubscriptionchecksmodel.h)

add_library(hydownloader-cpp::hydownloader-cpp ALIAS hydownloader-cpp)

if(USE_QT5)
    target_link_libraries(hydownloader-cpp PRIVATE Qt5::Core Qt5::Network Qt5::Gui)
else()
    target_link_libraries(hydownloader-cpp PRIVATE Qt6::Core Qt6::Network Qt6::Gui)
endif()

install(TARGETS hydownloader-cpp LIBRARY DESTINATION lib)
