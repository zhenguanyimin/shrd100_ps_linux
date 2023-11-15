include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/pub)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/hal)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/cmdproc)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/thread)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/sys)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)

aux_source_directory(. DIR_LIB_SRCS)
add_library(data_recorder ${DIR_LIB_SRCS})
set(DATA_RECORDER_LIBS data_recorder PARENT_SCOPE)


