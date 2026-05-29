set(TARGET_NAME channel_ros1)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  nav_msgs
  sensor_msgs
  geometry_msgs
  tf
)
find_package(Eigen3 REQUIRED)

include_directories(${catkin_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIRS})

add_library(${TARGET_NAME} SHARED
  qnode.cpp
  qnode.h
  channel_symbol.cpp
  ../virtual_channel.h
)

set_target_properties(${TARGET_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/ros1
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_include_directories(${TARGET_NAME}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${TARGET_NAME}
  Qt5::Core
  ${catkin_LIBRARIES}
)
