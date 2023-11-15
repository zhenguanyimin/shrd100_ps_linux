include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)

# 
file(GLOB_RECURSE LIB_SOURCES_VEHICLEMOUNTED "*.cpp")
#
set(LIB_SOURCES_VEHICLEMOUNTED ${LIB_SOURCES_VEHICLEMOUNTED} PARENT_SCOPE)
