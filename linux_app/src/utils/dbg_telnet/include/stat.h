/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __STAT_H__
#define  __STAT_H__

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

static inline void sum_stats(uint64_t *dest, uint64_t *srcs[], int src_num, int item_num)
{

	int i, j;
	int stat_structure_size = sizeof(uint64_t)*item_num;

	memset(dest, 0, stat_structure_size);

	for (i=0; i<item_num; i++)
		for (j=0; j<src_num; j++)
			dest[i] += srcs[j][i];


}

static inline void sub_stats(uint64_t *dest, uint64_t *larger, uint64_t *smaller, int item_num)
{

	int i, j;
	int stat_structure_size = sizeof(uint64_t)*item_num;

	for (i=0; i<item_num; i++)
		dest[i] = larger[i] - smaller[i];


}

static inline void print_stats(int no_prt_zero_item, const uint64_t *items, const char *item_names[], int item_num)
{

	int i;
	printf("\n");

	for (i=0; i<item_num; i++) {
		if (items[i] || !no_prt_zero_item)
			printf("%-32s    0x%-16"PRIx64"(%"PRIu64")\n"
			,item_names[i]
			,items[i], items[i]);
	}
	fflush(stdout);

}
#endif

