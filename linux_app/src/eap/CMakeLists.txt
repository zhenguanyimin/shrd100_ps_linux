add_subdirectory(base)
add_subdirectory(os_adapter)
add_subdirectory(alink)
add_subdirectory(remoteid)
add_subdirectory(resp_msg)
add_subdirectory(heartbeat)
add_subdirectory(droneid)
add_subdirectory(frequency)
add_subdirectory(spectrum)
add_subdirectory(drone_sniffer)
add_subdirectory(airborne_sniffer)
add_subdirectory(alink_pb)
add_subdirectory(bluetooth)
add_subdirectory(vehicle_mounted)
#add_subdirectory(algorithm_stub)


include_directories(include)
include_directories(include/cmdproc)
include_directories(include/hal)
include_directories(include/module)
include_directories(include/pub)
include_directories(include/sys)
include_directories(include/thread)
include_directories(include/os_adapter)
include_directories(include/bluetooth)
include_directories(include/vehicle_mounted)
include_directories(${CMAKE_SOURCE_DIR}/src/utils/checksum)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
include_directories(../utils/data_recorder)
include_directories(../utils/rpt_ctrl)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg/det_alg)
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg/droneID)
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg/tracking)
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/alink_pb)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/freqdetect)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/include/warning)
include_directories(${CMAKE_SOURCE_DIR}/src/tracer)
include_directories(${CMAKE_SOURCE_DIR}/src/tracer/hal)
include_directories(${CMAKE_SOURCE_DIR}/src/tracer/hal/dma)

link_directories(${CMAKE_SOURCE_DIR}/../sysroot/usr/lib/)

set(CMAKE_EAP_LIBRARY_OUTPUT_DIRECTORY ../)
add_library(eap SHARED ${LIB_SOURCES_ALINK_PB} ${LIB_SOURCES_ALINK} ${LIB_SOURCES_BASE} ${LIB_SOURCES_DRONE_SNIFFER} ${LIB_SOURCES_AIRBORNE_SNIFFER} ${LIB_SOURCES_DRONEID} ${LIB_SOURCES_FREQUENCY} ${LIB_SOURCES_HEARTBEAT} ${LIB_SOURCES_OS_ADAPTER} ${LIB_SOURCES_REMOTEID} ${LIB_SOURCES_REAP_MSG} ${LIB_SOURCES_DRONE_SPECTRUM} ${LIB_SOURCES_ALGORITHM_ALG} ${LIB_SOURCES_BLUETOOTH} ${LIB_SOURCES_VEHICLEMOUNTED})
set_target_properties(eap PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_EAP_LIBRARY_OUTPUT_DIRECTORY})
#set(EAP_LIBS ${BASE_LIBS} ${OS_ADAPTER_LIBS} ${ALINK_LIBS} ${REMOTEID_LIBS} ${RESP_MSG_LIBS} ${HEARTBEAT_LIBS} ${DRONEID_LIBS} ${FREQUENCY_LIBS} ${SPECTRUM_LIBS} ${DRONE_SNIFFER_LIBS}PARENT_SCOPE)

link_libraries(../app/libapp.a)
target_link_libraries(eap protobuf sqlite3)
