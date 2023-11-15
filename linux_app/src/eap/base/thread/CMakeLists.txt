include_directories(../../include/cmdproc)
include_directories(../../include/hal)
include_directories(../../include/module)
include_directories(../../include/pub)
include_directories(../../include/sys)
include_directories(../../include/thread)
include_directories(../../include/os_adapter)
include_directories(../../../srv/cli)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)


aux_source_directory(. DIR_LIB_SRCS)
add_library(thread ${DIR_LIB_SRCS})
set (THREAD_BASE_LIBS thread PARENT_SCOPE)
