include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)


# 获取子目录中的所有源文件
file(GLOB_RECURSE LIB_SOURCES_REAP_MSG "*.cpp")
# 将源文件返回给父目录
set(LIB_SOURCES_REAP_MSG ${LIB_SOURCES_REAP_MSG} PARENT_SCOPE)