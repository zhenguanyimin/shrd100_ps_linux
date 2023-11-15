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
include_directories(${CMAKE_SOURCE_DIR}/src/app/alg)


# ��ȡ��Ŀ¼�е�����Դ�ļ�
file(GLOB_RECURSE LIB_SOURCES_DRONE_SPECTRUM "*.cpp")
# ��Դ�ļ����ظ���Ŀ¼
set(LIB_SOURCES_DRONE_SPECTRUM ${LIB_SOURCES_DRONE_SPECTRUM} PARENT_SCOPE)
