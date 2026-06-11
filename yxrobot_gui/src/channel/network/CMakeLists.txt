set(TARGET_NAME channel_network)

find_package(Protobuf REQUIRED)

set(NETWORK_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(NETWORK_PROTO_FILES
  base_proto/common.proto
  base_proto/geometry.proto
  base_proto/map.proto
  sensor_proto/smart_sensors.proto
)

set(NETWORK_PROTO_SRCS)
set(NETWORK_PROTO_HDRS)
foreach(PROTO_FILE ${NETWORK_PROTO_FILES})
  get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
  set(PROTO_SRC ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_DIR}/${PROTO_NAME}.pb.cc)
  set(PROTO_HDR ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_DIR}/${PROTO_NAME}.pb.h)
  add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${NETWORK_PROTO_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      ${NETWORK_PROTO_DIR}/${PROTO_FILE}
    DEPENDS ${NETWORK_PROTO_DIR}/${PROTO_FILE}
    COMMENT "Running cpp protocol buffer compiler on ${PROTO_FILE}"
    VERBATIM
  )
  list(APPEND NETWORK_PROTO_SRCS ${PROTO_SRC})
  list(APPEND NETWORK_PROTO_HDRS ${PROTO_HDR})
endforeach()

set_source_files_properties(${NETWORK_PROTO_SRCS} ${NETWORK_PROTO_HDRS}
  PROPERTIES SKIP_AUTOGEN ON
)

add_library(${TARGET_NAME} SHARED
  network_channel.cpp
  network_channel.h
  telemetry_protocol.h
  channel_symbol.cpp
  ../virtual_channel.h
  ${NETWORK_PROTO_SRCS}
  ${NETWORK_PROTO_HDRS}
)

set_target_properties(${TARGET_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/network
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_include_directories(${TARGET_NAME}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/proto
)

target_link_libraries(${TARGET_NAME}
  Qt5::Core
  Threads::Threads
  ${Protobuf_LIBRARIES}
)

install(TARGETS ${TARGET_NAME}
  LIBRARY DESTINATION lib/network
)
