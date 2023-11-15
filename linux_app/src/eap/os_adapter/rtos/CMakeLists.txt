include_directories(../../include/os_adapter)
include_directories(../../include/pub)

aux_source_directory(. DIR_LIB_SRCS)
add_library(base ${DIR_LIB_SRCS})
set (RTOS_ADAPTER_LIBS base PARENT_SCOPE)
