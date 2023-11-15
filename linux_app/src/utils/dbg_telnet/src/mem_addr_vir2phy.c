/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"
#include "log.h"

#define    page_map_file     "/proc/self/pagemap"
#define    PFN_MASK          ((((uint64_t)1)<<55)-1)
#define    PFN_PRESENT_FLAG  (((uint64_t)1)<<63)

int mem_addr_vir2phy(unsigned long vir, unsigned long *phy)
{
    int fd;
    int page_size=getpagesize();
    unsigned long vir_page_idx = vir/page_size;
    unsigned long pfn_item_offset = vir_page_idx*sizeof(uint64_t);
    uint64_t pfn_item;
    
    fd = open(page_map_file, O_RDONLY);
    if (fd<0)
    {
        ErrSysLog("open %s failed", page_map_file);
        return -1;
    }

    if ((off_t)-1 == lseek(fd, pfn_item_offset, SEEK_SET))
    {
        ErrSysLog("lseek %s failed", page_map_file);
        return -1;
    }

    if (sizeof(uint64_t) != read(fd, &pfn_item, sizeof(uint64_t)))
    {
        ErrSysLog("read %s failed", page_map_file);
        return -1;
    }

    if (0==(pfn_item & PFN_PRESENT_FLAG))
    {
        ErrSysLog("page is not present");
        return -1;
    }

    *phy = (pfn_item & PFN_MASK)*page_size + vir % page_size;
    return 0;

}

/*
�������vir��ַ��Ӧ��ҳ�治���ڴ��У������ڵ���mem_addr_vir2phy֮ǰ��
�ȷ���һ�´˵�ַ��

���磬 int  a=*(int *)(void *)vir;

�������Linux��swap���ܽ����̵�ҳ�潻����Ӳ���ϴӶ�����ҳ��������ַ�仯��
���Թر�swap���ܡ�

��������C�⺯��������ֹLinux����ǰ���̵Ĳ��ֻ�ȫ��ҳ�潻����Ӳ���ϡ�

       int mlock(const void *addr, size_t len);
       int mlockall(int flags);
*/