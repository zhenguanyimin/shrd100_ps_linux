add_subdirectory(checksum)
add_subdirectory(data_recorder)
add_subdirectory(lzma)
add_subdirectory(dbg_telnet)
add_subdirectory(instrumentor)
add_subdirectory(rpt_ctrl)

set(UTILS_LIBS 
    ${CHECKSUM_LIBS}
    ${INSTRUMENTOR_LIBS}
    ${LZMA_LIBS}
    ${DATA_RECORDER_LIBS}
    ${DBG_TELNET_LIBS}
    ${RPT_CTRL_LIBS}
    PARENT_SCOPE)
