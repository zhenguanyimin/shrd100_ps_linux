#
#
#aux_source_directory(. DIR_SCRS)

add_compile_options(-Wno-error=date-time)

if (PRODUCT)    
    #aux_source_directory(. DIR_LIB_SRCS)
    #add_library(main ${DIR_LIB_SRCS})
    
    #make_directory({$CMAKE_BINARY_DIR}/lib)
    #set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
    #general the target file
    add_subdirectory(app)
    add_subdirectory(eap)
    add_subdirectory(tracer)
    add_subdirectory(srv)
    add_subdirectory(utils)

    include_directories(eap/include/cfg)
    include_directories(eap/alink_pb)
    include_directories(eap/include/cmdproc)
    include_directories(eap/include/hal)
    include_directories(eap/include/module)
    include_directories(eap/include/pub)
    include_directories(eap/include/sys)
    include_directories(eap/include/thread)
    include_directories(eap/include/os_adapter)
    include_directories(eap/include/freqdetect)
    include_directories(eap/include/alink_pb)
    include_directories(srv/cli)
    include_directories(srv/log)
    include_directories(srv/wit)
    include_directories(srv/bluetooth)
    include_directories(srv/heartbeat_packet)
    include_directories(utils/checksum)
    include_directories(utils/instrumentor)
    include_directories(utils/data_recorder)
    include_directories(utils/lzma)
    include_directories(utils/rpt_ctrl)
    include_directories(app/alg/droneID)
    include_directories(app/alg/det_alg)
    include_directories(app/alg)
    include_directories(app/remote_id)
    include_directories(tracer)
    include_directories(tracer/hal)
    include_directories(tracer/hal/i2c)
    include_directories(tracer/hal/c2_network)
    include_directories(tracer/hal/c2_alink)
    include_directories(tracer/hal/c2_alink/analysis)
    include_directories(tracer/hal/c2_alink/check)
    include_directories(tracer/hal/c2_alink/upload)
    include_directories(tracer/hal/eth_link)
    include_directories(tracer/hal/eth_link/socket)
    include_directories(../../sysroot/usr/include)
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
    
    link_directories(/home/root/)
    link_directories(../../sysroot/usr/lib/)
    add_executable(${PROJECT_NAME}.elf main.cpp) 
    target_link_libraries(${PROJECT_NAME}.elf 
    -Wl,--start-group 
    #${EAP_LIBS} 
    ${APP_LIBS}
    ${SRV_LIBS}
    ${TRACER_LIBS}
    ${UTILS_LIBS}
    pthread
    rt
    pcap
    eap
    ev
    dl
    -Wl,--end-group
    )
else()
  include_directories(eap/include/cfg)
  include_directories(eap/alink_pb)
  include_directories(eap/include)
  include_directories(eap/include/cmdproc)
  include_directories(eap/include/cmdproc)
  include_directories(eap/include/hal)
  include_directories(eap/include/module)
  include_directories(eap/include/pub)
  include_directories(eap/include/sys)
  include_directories(eap/include/thread)
  include_directories(eap/include/os_adapter)
  include_directories(eap/include/freqdetect)
  include_directories(eap/include/warning)
  include_directories(eap/alink_pb)
  include_directories(srv/cli)
  include_directories(srv/log)
  include_directories(srv/wit)
  include_directories(srv/bluetooth)
  include_directories(srv/heartbeat_packet)
  include_directories(utils/checksum)
  include_directories(utils/instrumentor)
  include_directories(utils/data_recorder)
  include_directories(utils/lzma)
  include_directories(utils/dbg_telnet/include)
  include_directories(utils/rpt_ctrl)
  include_directories(app/alg/droneID)
  include_directories(app/alg/det_alg)
  include_directories(app/alg)
  include_directories(app/remote_id)
  include_directories(tracer)
  include_directories(../unittest/stub/hal)
  
  make_directory(${CMAKE_BINARY_DIR}/lib)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
  
  set(src_NAME ${PROJECT_NAME} CACHE STRING "src" FORCE)

  aux_source_directory(./eap/base/cfg base_cfg)
  aux_source_directory(./eap/base/cmdproc base_cmdproc)
  aux_source_directory(./eap/base/hal base_hal)
  aux_source_directory(./eap/base/module base_module)
  aux_source_directory(./eap/base/sys base_sys)
  aux_source_directory(./eap/base/thread base_thread)
  aux_source_directory(./eap/os_adapter/base os_adapter_base)
  aux_source_directory(./eap/alink eap_alink)
  aux_source_directory(./eap/remoteid eap_remoteid)
  aux_source_directory(./eap/resp_msg eap_resp_msg)
  aux_source_directory(./eap/droneid eap_droneid)
  aux_source_directory(./eap/heartbeat eap_heartbeat)
  aux_source_directory(./eap/frequency eap_frequency)
  aux_source_directory(./eap/spectrum eap_spectrum)
  aux_source_directory(./eap/drone_sniffer drone_sniffer)
  aux_source_directory(./eap/alink_pb alink_pb)

  aux_source_directory(./srv/cli cli)
  aux_source_directory(./srv/log log)
  aux_source_directory(./srv/wit wit)
  
  aux_source_directory(./utils/checksum checksum)
  aux_source_directory(./utils/instrumentor instrumentor)
  aux_source_directory(./utils/data_recorder data_recorder)
  aux_source_directory(./utils/lzma lzma)
  aux_source_directory(./utils/rpt_ctrl rpt_ctrl)
  
  aux_source_directory(./tracer tracer)
  aux_source_directory(./tracer/hal/aaps_info aaps_info)
  aux_source_directory(./tracer/hal/i2c i2c)
  aux_source_directory(./tracer/hal/tmp75c tmp75c)
  aux_source_directory(./tracer/hal/ads1115 ads1115)
  aux_source_directory(./tracer/hal/libattery libattery)
  

  add_library(${src_NAME} STATIC
  ${base_cfg} ${base_cmdproc} ${base_hal} ${base_module} ${base_sys} ${base_thread} ${os_adapter_base}
  ${eap_alink} ${eap_remoteid} ${eap_resp_msg} ${eap_heartbeat} ${eap_droneid} ${eap_frequency} 
  ${eap_spectrum} ${drone_sniffer} ${alink_pb}
  ${checksum} ${instrumentor} ${data_recorder} ${lzma} ${rpt_ctrl}
  ${cli} ${log} ${tracer} ${alink} ${aaps_info} ${i2c} ${tmp75c} ${ads1115} ${libattery})

endif(PRODUCT)
