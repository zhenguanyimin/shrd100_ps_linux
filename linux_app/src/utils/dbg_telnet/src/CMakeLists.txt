include_directories(${CMAKE_SOURCE_DIR}/src/utils/dbg_telnet/include)

aux_source_directory(. DIR_LIB_SRCS)
add_library(dbg_telnet ${DIR_LIB_SRCS})
set(DBG_TELNET_SRC_LIBS dbg_telnet PARENT_SCOPE)


