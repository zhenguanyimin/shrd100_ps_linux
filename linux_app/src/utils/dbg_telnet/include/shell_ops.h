/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __SHELL_OPS_H__
#define  __SHELL_OPS_H__

int cmd2file(const char *cmd, char *file_path);
int get_cmd_result_int(const char *cmd, int *result_code);
int get_cmd_result(char *buf, int buflen, const char *cmd);

#endif

