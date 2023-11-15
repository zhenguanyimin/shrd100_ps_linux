include_directories(../../include/os_adapter)
include_directories(../../include/pub)

aux_source_directory(. DIR_LIB_SRCS)
add_library(os_base ${DIR_LIB_SRCS})
set (OS_ADAPTER_BASE_LIBS os_base PARENT_SCOPE)
