include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/pub)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/cfg)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/alink_pb)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/os_adapter)

aux_source_directory(. DIR_LIB_SRCS)
add_library(rpt_ctrl ${DIR_LIB_SRCS})
set(RPT_CTRL_LIBS rpt_ctrl PARENT_SCOPE)


