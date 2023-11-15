/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "debug.h"

int map_memory(unsigned long long phy_addr, unsigned len, void **result)
{

	int ret = 0;
	void *vaddr;

	int fd;

	fd = open("dev/mem", O_RDWR);
#ifdef __LP64__
	vaddr = mmap(NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr);
#else
	vaddr = mmap64(NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr);
#endif
	*result = vaddr;

	return ret;

}
