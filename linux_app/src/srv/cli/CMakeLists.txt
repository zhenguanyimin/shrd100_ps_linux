#
#
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/pub)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
aux_source_directory(. DIR_LIBS_SRCS)
add_library(cli ${DIR_LIBS_SRCS})
set(CLI_LIBS cli PARENT_SCOPE)

