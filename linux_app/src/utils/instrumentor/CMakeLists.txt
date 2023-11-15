aux_source_directory(. DIR_LIB_SRCS)
add_library(instrumentor ${DIR_LIB_SRCS})
set(INSTRUMENTOR_LIBS instrumentor PARENT_SCOPE)


