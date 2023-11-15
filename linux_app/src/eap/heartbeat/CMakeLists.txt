include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/warning)

# 获取子目录中的所有源文件
file(GLOB_RECURSE LIB_SOURCES_HEARTBEAT "*.cpp")
# 将源文件返回给父目录
set(LIB_SOURCES_HEARTBEAT ${LIB_SOURCES_HEARTBEAT} PARENT_SCOPE)
