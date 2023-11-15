
add_subdirectory(cfg)
add_subdirectory(cmdproc)
add_subdirectory(hal)
add_subdirectory(module)
add_subdirectory(sys)
add_subdirectory(thread)

#set(BASE_LIBS ${CFG_BASE_LIBS} ${SYS_BASE_LIBS} ${CMDPROC_BASE_LIBS} ${HAL_BASE_LIBS} ${MODULE_BASE_LIBS} ${THREAD_BASE_LIBS} PARENT_SCOPE)

# ��ȡ���Ƕ��Ŀ¼��Դ�ļ�
file(GLOB_RECURSE LIB_SOURCES_BASE "cfg/*.cpp" "cmdproc/*.cpp" "hal/*.cpp" "module/*.cpp" "sys/*.cpp" "thread/*.cpp")
# ��Դ�ļ����ظ���Ŀ¼
set(LIB_SOURCES_BASE ${LIB_SOURCES_BASE} PARENT_SCOPE)