#
#

include_directories(${CMAKE_SOURCE_DIR}/src/tracer)
include_directories(${CMAKE_SOURCE_DIR}/src/tracer/hal/i2c)

aux_source_directory(. DIR_LIBS_SRCS)
add_library(usb_typec ${DIR_LIBS_SRCS})
set(USB_TYPEC_LIBS usb_typec PARENT_SCOPE)
