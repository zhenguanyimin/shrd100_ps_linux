add_subdirectory(cli)
add_subdirectory(log)
add_subdirectory(wit)
add_subdirectory(bluetooth)

#aux_source_directory(. DIR_LIB_SRCS)
#add_library(main ${DIR_LIB_SRCS})
set(SRV_LIBS ${CLI_LIBS} 
    ${LOG_LIBS}
    ${WIT_LIBS}
    ${BLUETOOTH_LIBS}
    PARENT_SCOPE)
