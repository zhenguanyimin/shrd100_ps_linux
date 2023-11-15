/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */


/* ������-������ ����*/
#ifndef  __PC_QUEUE_H__
#define __PC_QUEUE_H__

#include <stdint.h>


typedef struct
{
void *                for_align;
volatile    uint32_t  valid;
volatile    int32_t   para_int;
    void    *volatile para;
    void    *volatile data;
} t_pc_que_entry;

typedef struct
{
void *                for_align;
    uint32_t          nr_entry;
    t_pc_que_entry    *pt_head;
    t_pc_que_entry    *pt_tail;
    t_pc_que_entry    *pt_p_ptr;
    t_pc_que_entry    *pt_c_ptr;
    t_pc_que_entry    p_at_entries[0];

} t_pc_que;

void* create_pc_que(uint32_t size);

void* pc_que_outq_try(t_pc_que *pt_q, int32_t   *para_int, void **para);
int   pc_que_enq_try(t_pc_que *pt_q, void *data, int32_t   para_int, void *para);
#endif

