set(TARGET_NAME channel_network)

set(ROBOT536_DIR "/home/suj/Desktop/yx_work/mstf/modules/robot536" CACHE PATH "robot536 source directory")

find_package(Protobuf REQUIRED)

add_library(${TARGET_NAME} SHARED
  network_channel.cpp
  network_channel.h
  telemetry_protocol.h
  channel_symbol.cpp
  ../virtual_channel.h
)

set_target_properties(${TARGET_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/network
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_include_directories(${TARGET_NAME}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${ROBOT536_DIR}/common/proto_cxx
    ${ROBOT536_DIR}/common/proto_cxx/base_proto
    ${ROBOT536_DIR}/common/proto_cxx/sensor_proto
)

target_link_libraries(${TARGET_NAME}
  Qt5::Core
  Threads::Threads
  ${ROBOT536_DIR}/build/common/proto/libsilverstar_proto.a
  ${Protobuf_LIBRARIES}
)
