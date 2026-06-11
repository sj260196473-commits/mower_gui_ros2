cmake_minimum_required(VERSION 3.8)
project(yxrobot_gui)

# 选择通信通道：none / ros1 / ros2 / network
set(CHANNEL_OPTION "network" CACHE STRING "Communication channel to build: none, ros1, ros2 or network")
set_property(CACHE CHANNEL_OPTION PROPERTY STRINGS none ros1 ros2 network)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
find_package(Threads REQUIRED)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow/ui)


find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Eigen3 REQUIRED)
include(CTest)

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
  src/common/map_coordinate_transformer.h
  src/common/point.h
  src/mainwindow/mainwindow.h
  src/mainwindow/map_panel/core/mapdisplay_factory.h
  src/mainwindow/map_panel/core/editable_zone_model.h
  src/mainwindow/map_panel/core/pnc_task_model.h
  src/mainwindow/map_panel/core/map_layer_registry.h
  src/mainwindow/map_panel/core/map_layer_runtime.h
  src/mainwindow/map_panel/core/map_layeritem_virtual.h
  src/mainwindow/map_panel/layers/costmap_layerItem.h
  src/mainwindow/map_panel/layers/editable_zone_layeritem.h
  src/mainwindow/map_panel/layers/grid_layeritem.h
  src/mainwindow/map_panel/layers/laser_layeritem.h
  src/mainwindow/map_panel/layers/occmap_layerItem.h
  src/mainwindow/map_panel/layers/robotpose_layerItem.h
  src/mainwindow/map_panel/layers/path_layerItem.h
  src/mainwindow/map_panel/view/map_graphicsview.h
  src/mainwindow/map_panel/view/map_overlay_widget.h
)

set(PRO_SOURCES_DIR
  src/app/main.cpp
  src/channel/channel_manager.cpp
  src/mainwindow/mainwindow.cpp
  src/mainwindow/map_panel/core/mapdisplay_factory.cpp
  src/mainwindow/map_panel/core/editable_zone_model.cpp
  src/mainwindow/map_panel/core/pnc_task_model.cpp
  src/mainwindow/map_panel/core/map_layer_registry.cpp
  src/mainwindow/map_panel/core/map_layer_runtime.cpp
  src/mainwindow/map_panel/core/map_layerItem_virtual.cpp
  src/mainwindow/map_panel/layers/costmap_layerItem.cpp
  src/mainwindow/map_panel/layers/editable_zone_layeritem.cpp
  src/mainwindow/map_panel/layers/grid_layeritem.cpp
  src/mainwindow/map_panel/layers/laser_layeritem.cpp
  src/mainwindow/map_panel/layers/occmap_layerItem.cpp
  src/mainwindow/map_panel/layers/robotpose_layerItem.cpp
  src/mainwindow/map_panel/layers/path_layerItem.cpp
  src/mainwindow/map_panel/view/map_graphicsview.cpp
  src/mainwindow/map_panel/view/map_overlay_widget.cpp
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
  Threads::Threads
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN3_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  YX_CHANNEL_OPTION="${CHANNEL_OPTION}"
)

if(NOT CHANNEL_OPTION STREQUAL "none"
   AND NOT CHANNEL_OPTION STREQUAL "ros1"
   AND NOT CHANNEL_OPTION STREQUAL "ros2"
   AND NOT CHANNEL_OPTION STREQUAL "network")
  message(FATAL_ERROR "Unsupported CHANNEL_OPTION='${CHANNEL_OPTION}'. Expected 'none', 'ros1', 'ros2' or 'network'.")
endif()

if(CHANNEL_OPTION STREQUAL "ros2")
  add_subdirectory(src/channel/ros2)
elseif(CHANNEL_OPTION STREQUAL "ros1")
  add_subdirectory(src/channel/ros1)
elseif(CHANNEL_OPTION STREQUAL "network")
  add_subdirectory(src/channel/network)
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .
)

get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
if(QT_QMAKE_EXECUTABLE)
  execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_INSTALL_PLUGINS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  foreach(QT_PLATFORM_PLUGIN libqxcb.so libqoffscreen.so libqminimal.so)
    set(QT_PLATFORM_PLUGIN_PATH "${QT_INSTALL_PLUGINS}/platforms/${QT_PLATFORM_PLUGIN}")
    if(EXISTS "${QT_PLATFORM_PLUGIN_PATH}")
      install(FILES "${QT_PLATFORM_PLUGIN_PATH}" DESTINATION plugins/platforms)
    endif()
  endforeach()
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/packaging/run_gui.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/run.sh
  @ONLY
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run.sh DESTINATION .)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/packaging/qt.conf
  DESTINATION .
)

if(BUILD_TESTING)
  add_executable(test_editable_zone_model
    tests/test_editable_zone_model.cpp
    src/mainwindow/map_panel/core/editable_zone_model.cpp
  )
  target_include_directories(test_editable_zone_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  target_link_libraries(test_editable_zone_model
    Qt5::Core
  )
  add_test(NAME test_editable_zone_model COMMAND test_editable_zone_model)

  add_executable(test_pnc_task_model
    tests/test_pnc_task_model.cpp
    src/mainwindow/map_panel/core/pnc_task_model.cpp
    src/mainwindow/map_panel/core/editable_zone_model.cpp
  )
  target_include_directories(test_pnc_task_model PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  target_link_libraries(test_pnc_task_model
    Qt5::Core
  )
  add_test(NAME test_pnc_task_model COMMAND test_pnc_task_model)

  qt5_wrap_cpp(TEST_ROBOTPOSE_MOC_HPP
    src/mainwindow/map_panel/layers/robotpose_layerItem.h
    src/mainwindow/map_panel/core/map_layeritem_virtual.h
  )
  add_executable(test_robotpose_layeritem
    tests/test_robotpose_layeritem.cpp
    src/mainwindow/map_panel/layers/robotpose_layerItem.cpp
    src/mainwindow/map_panel/core/map_layerItem_virtual.cpp
    ${TEST_ROBOTPOSE_MOC_HPP}
  )
  target_include_directories(test_robotpose_layeritem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${EIGEN3_INCLUDE_DIRS}
  )
  target_link_libraries(test_robotpose_layeritem
    Qt5::Core
    Qt5::Widgets
  )
  add_test(NAME test_robotpose_layeritem COMMAND test_robotpose_layeritem)
endif()
