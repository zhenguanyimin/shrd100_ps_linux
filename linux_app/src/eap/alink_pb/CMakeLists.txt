include_directories(../include)
include_directories(../include/cfg)
include_directories(${CMAKE_SOURCE_DIR}/src/eap/alink_pb)

include_directories(../include/cmdproc)
include_directories(../include/hal)
include_directories(../include/module)
include_directories(../include/pub)
include_directories(../include/sys)
include_directories(../include/thread)
include_directories(../include/os_adapter)
include_directories(${CMAKE_SOURCE_DIR}/src/utils/checksum)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/log)
include_directories(${CMAKE_SOURCE_DIR}/src/srv/cli)

# ��ȡ��Ŀ¼�е�����Դ�ļ�
file(GLOB_RECURSE LIB_SOURCES_ALINK_PB "*.cc")
# ��Դ�ļ����ظ���Ŀ¼
set(LIB_SOURCES_ALINK_PB ${LIB_SOURCES_ALINK_PB} PARENT_SCOPE)


