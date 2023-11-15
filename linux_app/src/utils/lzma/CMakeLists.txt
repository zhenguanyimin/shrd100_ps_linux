aux_source_directory(. DIR_LIB_SRCS)
add_library(lzma ${DIR_LIB_SRCS})
set(LZMA_LIBS lzma PARENT_SCOPE)

