/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __LOG_H__
#define  __LOG_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int get_log_file_fd();

#define    DFT_LOG_FILE_SIZE    (4096)
int init_log(char *file_name, int file_size /* bytes */);
int write_log(const char *fmt, ...);

#define SysLog(fmt, args...) \
    do \
    { \
        printf("DBG:%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
	fflush(stdout); \
        write_log("%s(%d), %s:\n"fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args);\
    } while (0)


#define ErrSysLog(fmt, args...) \
    do \
    { \
        printf("ERROR:%s(%d)-%s:\n"fmt"\n:%s\n", __FILE__,__LINE__,__FUNCTION__,##args, strerror(errno)); \
	fflush(stdout); \
        write_log("ERROR:%s(%d), %s:\n"fmt": %s\n", __FILE__, __LINE__, __FUNCTION__, ##args, strerror(errno));\
    } while (0)

#define ErrSysLogQuit(fmt, args...) \
    do \
    { \
        printf("ErrQuit:%s(%d)-%s:\n"fmt"\n:%s\n", __FILE__,__LINE__,__FUNCTION__,##args, strerror(errno)); \
	fflush(stdout); \
        write_log("ErrQuit: %s(%d), %s:\n"fmt": %s\n", __FILE__, __LINE__, __FUNCTION__, ##args, strerror(errno));\
        exit(1);\
    } while (0)

#endif

