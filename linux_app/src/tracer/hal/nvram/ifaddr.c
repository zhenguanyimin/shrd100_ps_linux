/**
 * @file nvram_cust.c
 * @brief nvram driver.
 *
 * 
 * @author fanghaohua
 * @date 2023.09.26
 * @see 
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
//#include <termios.h>
//#include "eap_sys_drvcodes.h"
//#include "eap_pub.h"

#include "ifaddr.h"

#define DEBUG_UNITTEST 1
#define DEBUG_NVRAM 1

#define IFNAME_WIFI_PATH "/tmp/ifname_wifi"
#define IFNAME_WIFI_DEFAULT "wlan0"
#define IFADDR_WIFI_DEFAULT "0.0.0.0"
//#define MAX_IFADDR_LENGTH 16
#define MIN(a, b) ((a)<(b) ? (a) : (b))

#if DEBUG_NVRAM
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif

static char s_ifname_wifi[IFNAMSIZ] = {0};

static int get_ifname_wifi()
{
    int result = 0;
    int fd = open(IFNAME_WIFI_PATH, O_RDONLY);
    if (0 > fd) {
        DEBUG_PRINTF("Open ifname failed! fd=%d\n", fd);
        goto exit_point;
    }

    memset(s_ifname_wifi, 0, IFNAMSIZ);
    result = read(fd, s_ifname_wifi, IFNAMSIZ);
    if (0 >= result) {
        result = MIN(strlen(IFNAME_WIFI_DEFAULT), IFNAMSIZ);
        strncpy(s_ifname_wifi, IFNAME_WIFI_DEFAULT, result);
    }
    DEBUG_PRINTF("%s: ifname_wifi=%s result=%d\n", __FUNCTION__, s_ifname_wifi, result);

exit_point:
    if (0 <= fd) {
        close(fd);
    }
    return result;
}

int get_ifaddr_wifi(uint8_t *addr, uint32_t len)
{
    int result = 0;
    int sockfd;

    if (0 >= strlen(s_ifname_wifi)) {
        result = get_ifname_wifi();
    }

    if (INET_ADDRSTRLEN > len) {
        DEBUG_PRINTF("Error parameter: len=%d\n", len);
        goto exit_point;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 > sockfd) {
        DEBUG_PRINTF("Create socket failed: sockfd=%d\n", sockfd);
        goto exit_point;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, s_ifname_wifi, IFNAMSIZ-1);
    result = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (0 > result) {
        DEBUG_PRINTF("ioctl SIOCGIFADDR failed: result=%d\n", result);
        goto exit_point;
       
    }
    struct sockaddr_in *sa_in = (struct sockaddr_in *)&ifr.ifr_addr;
    char if_addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sa_in->sin_addr, if_addr, sizeof(if_addr));
    result = MIN(len, INET_ADDRSTRLEN);
    strncpy(addr, if_addr, result);
    DEBUG_PRINTF("%s: addr=%s len=%d\n", __FUNCTION__, addr, result);

exit_point:
    if (0 <= sockfd) {
        close(sockfd);
    }
    return result;
}

#ifdef __cplusplus
}
#endif
