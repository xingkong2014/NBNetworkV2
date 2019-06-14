cmake_minimum_required(VERSION 3.14)

project(NBNetworkV2)

aux_source_directory(core DIR_LIB_SRCS)

add_library(NBNetworkV2 ${DIR_LIB_SRCS})