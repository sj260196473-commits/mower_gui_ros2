cmake_minimum_required(VERSION 3.8)
project(yxrobot_gui)

# 选择通信通道：none 或 ros2 暂时写死
# set(CHANNEL_OPTION "none" CACHE STRING "Communication channel to build: none or ros2")
# set_property(CACHE CHANNEL_OPTION PROPERTY STRINGS none ros2)
set(CHANNEL_OPTION "ros2")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow/ui)


find_package(Qt5 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)
find_package(Qt5 REQUIRED COMPONENTS Core)

set(UI_DIR
  src/mainwindow/ui/mainwindow.ui
)

set(PRO_RESOURCES_DIR
  src/mainwindow/resource/images.qrc
)

set(PRO_INCLUDE_DIR
  src/channel/channel_manager.h
  src/channel/virtual_channel.h
  src/common/common.h
  src/common/point.h
  src/mainwindow/mainwindow.h
  src/mainwindow/map_panel/costmap_layerItem.h
  src/mainwindow/map_panel/laser_layeritem.h
  src/mainwindow/map_panel/mapdisplay_factory.h
  src/mainwindow/map_panel/map_graphicsview.h
  src/mainwindow/map_panel/map_layeritem_virtual.h
  src/mainwindow/map_panel/occmap_layerItem.h
  src/mainwindow/map_panel/robotpose_layerItem.h
  src/mainwindow/map_panel/path_layerItem.h
)

set(PRO_SOURCES_DIR
  src/app/main.cpp
  src/channel/channel_manager.cpp
  src/mainwindow/mainwindow.cpp
  src/mainwindow/map_panel/costmap_layerItem.cpp
  src/mainwindow/map_panel/laser_layeritem.cpp
  src/mainwindow/map_panel/mapdisplay_factory.cpp
  src/mainwindow/map_panel/map_graphicsview.cpp
  src/mainwindow/map_panel/map_layerItem_virtual.cpp
  src/mainwindow/map_panel/occmap_layerItem.cpp
  src/mainwindow/map_panel/robotpose_layerItem.cpp
  src/mainwindow/map_panel/path_layerItem.cpp
)

qt5_wrap_ui(QT_UI_HPP ${UI_DIR})
qt5_wrap_cpp(QT_MOC_HPP ${PRO_INCLUDE_DIR})
qt5_add_resources(QT_RESOURCES_CPP ${PRO_RESOURCES_DIR})

add_executable(${PROJECT_NAME}
  ${PRO_SOURCES_DIR}
  ${QT_RESOURCES_CPP}
  ${PRO_INCLUDE_DIR}
  ${QT_MOC_HPP}
  ${QT_UI_HPP}
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Widgets
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow/map_panel>
  $<INSTALL_INTERFACE:include>
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  YX_CHANNEL_OPTION="${CHANNEL_OPTION}"
)

if(NOT CHANNEL_OPTION STREQUAL "none" AND NOT CHANNEL_OPTION STREQUAL "ros2")
  message(FATAL_ERROR "Unsupported CHANNEL_OPTION='${CHANNEL_OPTION}'. Expected 'none' or 'ros2'.")
endif()

if(CHANNEL_OPTION STREQUAL "ros2")
  add_subdirectory(src/channel/ros2)
endif()

