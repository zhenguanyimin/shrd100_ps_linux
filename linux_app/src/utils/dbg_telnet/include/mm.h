/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __MM_H__
#define  __MM_H__
#include <stdint.h>
typedef    void *    MM_HANDLE;
typedef int (*p_func)(void *buf);
MM_HANDLE create_buffer_manager(int buf_size, int num);
int  MM_HANDLE_IS_VALID(MM_HANDLE hd);
void free_buffer(MM_HANDLE hd, void *buf);
void *alloc_buffer(MM_HANDLE hd);
void add_to_all_link(MM_HANDLE hd, void *buf);
void delete_from_all_link(MM_HANDLE hd, void *buf);
void for_each_buf_in_all_link(MM_HANDLE hd, p_func usr_func);
uint32_t free_buffer_num(MM_HANDLE hd);
#endif

