/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __SERIAL_PORT_H__
#define  __SERIAL_PORT_H__

int serial_init(const char *serial_dev_name, int *p_fd, int RTSCTS, int speed, int need_line_input);
int serial_write(int fd_serial, void *src, int len);
int serial_read(int fd_serial, char *buf, int len);

#endif

