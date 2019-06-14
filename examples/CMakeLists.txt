cmake_minimum_required(VERSION 3.14)

project(example)

include_directories(../src)
link_directories(../src/build)

aux_source_directory(NServer DIR_SERVER_SRCS)
aux_source_directory(NClient DIR_CLIENT_SRCS)

add_executable(NServer ${DIR_SERVER_SRCS})
add_executable(NClient ${DIR_CLIENT_SRCS})

target_link_libraries(NServer NBNetworkV2 pthread)
target_link_libraries(NClient NBNetworkV2 pthread)