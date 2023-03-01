cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue, CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto graph.proto transport_router.proto)

set(COMMON_FILES headers/domain.h headers/geo.h headers/graph.h headers/ranges.h main.cpp transport_catalogue.proto map_renderer.proto graph.proto transport_router.proto)
set(READER_FILES headers/json_reader.h json_reader.cpp headers/json_builder.h json_builder.cpp)
set(HANLER_FILES headers/request_handler.h request_handler.cpp)
set(LIBRARY_FILES headers/json.h json.cpp headers/svg.h svg.cpp headers/serialization.h serialization.cpp)
set(ROUTE_FILES headers/router.h headers/map_renderer.h map_renderer.cpp headers/transport_router.h transport_router.cpp)
set(CATALOG_FILES headers/transport_catalogue.h transport_catalogue.cpp)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${COMMON_FILES} ${READER_FILES} ${HANLER_FILES} ${LIBRARY_FILES} ${ROUTE_FILES} ${CATALOG_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)