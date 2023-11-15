cmake_minimum_required(VERSION 3.0.0)
project(Main)
message("\nUnittest Main")
#message("pub_NAME is>>>${pub_NAME}\n")

string(REGEX REPLACE "(.*)/(.*)" "\\1" FATHER_DIR ${PROJECT_SOURCE_DIR})

aux_source_directory(./ SELF)
aux_source_directory(../tests tests)
aux_source_directory(../stub/alg alg_stub)
aux_source_directory(../stub/hal hal_stub)
aux_source_directory(../stub/hal/ad9361 hal_stub)
aux_source_directory(../stub/hal/gpio hal_stub)
aux_source_directory(../stub/hal/output hal_stub)
aux_source_directory(../stub/hal/input hal_stub)
aux_source_directory(../stub/hal/switch_filter hal_stub)
aux_source_directory(../stub/hal/c2_alink hal_stub)
aux_source_directory(../stub/hal/c2_network hal_stub)
aux_source_directory(../stub/hal/eth_link hal_stub)
aux_source_directory(../stub/hal/dma hal_stub)
aux_source_directory(../stub/hal/nvram hal_stub)
aux_source_directory(../stub/plt plt_stub)
aux_source_directory(../stub/drv drv_stub)
aux_source_directory(../stub/pub pub_stub)
aux_source_directory(../stub/alg alg_stub)
aux_source_directory(../stub/bluetooth bluetooth_stub)
aux_source_directory(../stub/other other_stub)

add_executable(main ${SELF} ${tests} ${alg_stub} ${hal_stub} ${plt_stub} ${drv_stub} ${pub_stub} ${alg_stub} ${bluetooth_stub} ${other_stub})

#target_link_libraries(main ${rf_config_NAME})
#target_link_libraries(main ${switch_filter_NAME})
#target_link_libraries(main ${eap_NAME})
#target_link_libraries(main ${cfg_NAME})
#target_link_libraries(main ${sd_app_NAME})
#target_link_libraries(main ${battery_NAME})
#target_link_libraries(main ${remote_id_NAME})
#target_link_libraries(
#	main
#	-Wl,--start-group
#	${data_path_NAME} ${alg_NAME} ${key_handle_NAME} ${srv_NAME} ${sd_app_NAME}
#	-Wl,--end-group
#)

add_definitions(-D__UNITTEST__)

target_link_libraries(
	main
	-Wl,--start-group
	${src_NAME}
	-Wl,--end-group
)

if(WIN32)
    find_library(protobuflib libprotobufd.a ${ROOT_DIR}/unittest/lib/protobuf)
    find_library(protobuflitelib libprotobuf-lited.a ${ROOT_DIR}/unittest/lib/protobuf)
    find_library(protobufdlib libprotocd.a ${ROOT_DIR}/unittest/lib/protobuf)
else()
    find_library(protobuflib libprotobuf_linux.a ${ROOT_DIR}/unittest/lib/protobuf)
    find_library(protobuflitelib libprotobuf-lite_linux.a ${ROOT_DIR}/unittest/lib/protobuf)
    find_library(protobufdlib libprotoc_linux.a ${ROOT_DIR}/unittest/lib/protobuf)
endif(WIN32)

target_link_libraries(main ${protobuflib})
target_link_libraries(main ${protobufdlib})
target_link_libraries(main ${protobuflitelib})

if(WIN32)
    find_library(gtestlib libgtest.a ${ROOT_DIR}/unittest/lib)
else()
    find_library(gtestlib libgtest_linux.a ${ROOT_DIR}/unittest/lib)
endif(WIN32)

target_link_libraries(main ${gtestlib})

include_directories(${ROOT_DIR}/unittest/stub/pub)

include_directories(${ROOT_DIR}/src/eap/include)
include_directories(${ROOT_DIR}/src/eap/include/cmdproc)
include_directories(${ROOT_DIR}/src/eap/include/hal)
include_directories(${ROOT_DIR}/src/eap/include/module)
include_directories(${ROOT_DIR}/src/eap/include/pub)
include_directories(${ROOT_DIR}/src/eap/include/sys)
include_directories(${ROOT_DIR}/src/eap/include/thread)
include_directories(${ROOT_DIR}/src/eap/include/os_adapter)
include_directories(${ROOT_DIR}/src/eap/include/freqdetect)
include_directories(${ROOT_DIR}/src/eap/include/warning)
include_directories(${ROOT_DIR}/src/eap/alink_pb)

include_directories(${ROOT_DIR}/src/srv/cli)
include_directories(${ROOT_DIR}/src/srv/log)
include_directories(${ROOT_DIR}/srv/heartbeat_packet)
include_directories(${ROOT_DIR}/src/tracer)

include_directories(${ROOT_DIR}/src/app/alg/droneID)
include_directories(${ROOT_DIR}/src/app/alg/det_alg)
include_directories(${ROOT_DIR}/src/app/alg)

include_directories(${ROOT_DIR}/src/utils/rpt_ctrl)

include_directories(../include)

make_directory(${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
