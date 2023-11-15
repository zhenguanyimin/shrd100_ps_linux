#
#
include_directories(${CMAKE_SOURCE_DIR}/src/srv/bluetooth)
aux_source_directory(. DIR_LIB_SRCS)
add_library(bluetooth ${DIR_LIB_SRCS})
set(BLUETOOTH_LIBS bluetooth PARENT_SCOPE)


#set(GOCAPI_LIB /home/a23315/work/0922_typec/shrd100_ps_linux/linux_app/src/srv/bluetooth/libgocapi.so)
#find_library(GOCAPI_LIB  gocapi)
set(GOCAPI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/libgocapi.so)
find_library(GOCAPI_LIB  gocapi)
if(GOCAPI_LIB)
	#link_libraries(${GOCAPI_LIB})
	target_link_libraries(bluetooth PRIVATE ${GOCAPI_LIB})
else()
  message(FATAL_ERROR "Cannot find libgocapi.so")
endif()



#link_libraries(../../../kernel/sdk_tracer_petalinux20222/project-spec/meta-user/recipes-apps/bluetooth/files/libgocapi.so)


