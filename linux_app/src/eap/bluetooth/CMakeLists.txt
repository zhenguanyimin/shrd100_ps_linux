include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)

# 
file(GLOB_RECURSE LIB_SOURCES_BLUETOOTH "*.cpp")
#
set(LIB_SOURCES_BLUETOOTH ${LIB_SOURCES_BLUETOOTH} PARENT_SCOPE)
