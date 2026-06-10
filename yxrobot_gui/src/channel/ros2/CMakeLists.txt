set(OpenCV_DIR /usr/lib/x86_64-linux-gnu/cmake/opencv4)
set(TARGET_NAME channel_ros2)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)

add_library(${TARGET_NAME} SHARED
  rclcomm.cpp
  rclcomm.h
  channel_symbol.cpp
  ../virtual_channel.h
)

set_target_properties(${TARGET_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ros2
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_include_directories(${TARGET_NAME}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${TARGET_NAME}
  Qt5::Core
)

ament_target_dependencies(${TARGET_NAME}
  rclcpp
  sensor_msgs
  geometry_msgs
  nav_msgs
  visualization_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  cv_bridge
  image_transport
)
