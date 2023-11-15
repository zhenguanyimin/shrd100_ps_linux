/* 
 * �����Ϊ��ѡ���Դ�����
 * ������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#ifndef  __SCREEN_OPS_H__
#define  __SCREEN_OPS_H__

#include <stdio.h>

// �����Ļ 
#define CLEAR_SCREEN() do { printf("\033[2J"); fflush(stdout); } while (0)

// ����ƶ� 
#define MOVE_CURSOR_UP(x) do { printf("\033[%dA", (x)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_DOWN(x) do { printf("\033[%dB", (x)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_LEFT(y) do { printf("\033[%dD", (y)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_RIGHT(y) do { printf("\033[%dC",(y)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_TO(x,y) do { printf("\033[%d;%dH", (x), (y)) ; fflush(stdout); } while (0)

// �������� 
#define RESET_CURSOR() do { printf("\033[H") ; fflush(stdout); } while (0)
#define HIDE_CURSOR() do { printf("\033[?25l") ; fflush(stdout); } while (0)
#define SHOW_CURSOR() do { printf("\033[?25h") ; fflush(stdout); } while (0)
#define HIGHT_LIGHT() do { printf("\033[7m")
#define UN_HIGHT_LIGHT() do { printf("\033[27m"); fflush(stdout); } while (0)
#define PUSH_SCREEN() do { printf("\033[?1049h\033[H"); fflush(stdout); } while (0)
#define POP_SCREEN() do { printf("\033[?1049l"); fflush(stdout); } while (0)

/* �����Ǹ������Ϣ */
#if 0
printf("\033[47;31mhello world\033[5m");

47���ֱ�����ɫ, 31���������ɫ, hello world���ַ���. �����\033[5m�ǿ�����.
��ɫ����:
QUOTE:
�ֱ�����ɫ��Χ: 40--49 ����ɫ: 30--39
40: �� 30: ��
41: �� 31: ��
42: �� 32: ��
43: �� 33: ��
44: �� 34: ��
45: �� 35: ��
46: ���� 36: ����
47: ��ɫ 37: ��ɫ
ANSI������:
QUOTE:
\033[0m �ر��������� 
\033[1m ���ø����� 
\03[4m �»��� 
\033[5m ��˸ 
\033[7m ���� 
\033[8m ���� 
\033[30m -- \033[37m ����ǰ��ɫ 
\033[40m -- \033[47m ���ñ���ɫ 
\033[nA �������n�� 
\03[nB �������n�� 
\033[nC �������n�� 
\033[nD �������n�� 
\033[y;xH���ù��λ�� 
\033[2J ���� 
\033[K ����ӹ�굽��β������ 
\033[s ������λ�� 
\033[u �ָ����λ�� 
\033[?25l ���ع�� 
\33[?25h ��ʾ���
����, ��ĳЩʱ��Ϳ���ʵ�ֶ�̬�����

#endif

#endif

