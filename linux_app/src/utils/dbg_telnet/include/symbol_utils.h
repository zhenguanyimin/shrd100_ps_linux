/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */


#ifndef  __SYMBOL_UTILS_H__
#define  __SYMBOL_UTILS_H__

#define _GNU_SOURCE
#include <dlfcn.h>


int init_symbol();
int addr2symbol_info(void *addr, Dl_info *info);
void proccess_cmd(char *cmd_line);

#endif