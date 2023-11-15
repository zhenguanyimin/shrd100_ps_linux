#
#
#aux_source_directory(. DIR_SCRS)

#add_subdirectory(alink)

include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/pub)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
aux_source_directory(. DIR_LIB_SRCS)
add_library(log ${DIR_LIB_SRCS})
set(LOG_LIBS log PARENT_SCOPE)


