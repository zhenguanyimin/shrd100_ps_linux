#
#
include_directories(${CMAKE_SOURCE_DIR}/src/srv/wit)
aux_source_directory(. DIR_LIB_SRCS)
add_library(wit ${DIR_LIB_SRCS})
set(WIT_LIBS wit PARENT_SCOPE)

