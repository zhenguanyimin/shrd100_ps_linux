aux_source_directory(. DIR_LIB_SRCS)
add_library(checksum ${DIR_LIB_SRCS})
set(CHECKSUM_LIBS checksum PARENT_SCOPE)


