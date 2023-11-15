include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/utils/rpt_ctrl)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg/droneID)


# ��ȡ��Ŀ¼�е�����Դ�ļ�
file(GLOB_RECURSE LIB_SOURCES_REMOTEID "*.cpp")
# ��Դ�ļ����ظ���Ŀ¼
set(LIB_SOURCES_REMOTEID ${LIB_SOURCES_REMOTEID} PARENT_SCOPE)