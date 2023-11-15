/**
 * @file nvram_cust.c
 * @brief nvram driver.
 *
 * 
 * @author fanghaohua
 * @date 2023.09.26
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termios.h>
//#include "eap_sys_drvcodes.h"
//#include "eap_pub.h"
#include <string.h>
#include "eap_product_features.h"
#include "eap_sys_cmds.h"

#include "nvram_cust.h"

#define DEBUG_UNITTEST 1
#define DEBUG_NVRAM 1

//#define NVRAM_PATH "/tmp/emmc1/nvram.rcd"
#define NVRAM_PATH "/run/media/mmcblk0p3/nvram.rcd"

#define MIN(a, b) ((a)<(b) ? (a) : (b))

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG_NVRAM
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif

static int nvram_check_sn()
{
    int result = access(NVRAM_PATH, F_OK);
    int fd = -1;
    char *sn = EAP_EMBED_DEFAULT_SN_CODE;
    int len = strlen(sn);

//    DEBUG_PRINTF("%s: default len=%d\n", __FUNCTION__, len);
    if (result != 0) {
        fd = open(NVRAM_PATH, O_WRONLY|O_CREAT, 0666);
        if (0 > fd) {
            DEBUG_PRINTF("Create nvram failed! fd=%d\n", fd);
            goto exit_point;
        }
        result = write(fd, sn, len);
        if (len != result) {
            DEBUG_PRINTF("Write default result=%d\n", result);
            goto exit_point;
        }
    }

exit_point:
    if (0 <= fd) {
        close(fd);
    }
    return result;
}

int nvram_read_sn(uint8_t *sn, uint32_t *len)
{
    int result = 0;
    char tmp_buf[EAP_SN_LENGTH];
    int read_len = MIN(*len, sizeof(tmp_buf));
    int fd = -1;

    if (0 >= len) {
        DEBUG_PRINTF("Error parameter: len=%d\n", *len);
        goto exit_point;
    }

    nvram_check_sn();

    fd = open(NVRAM_PATH, O_RDONLY);
    if (0 > fd) {
        DEBUG_PRINTF("Open nvram failed! fd=%d\n", fd);
        goto exit_point;
    }

    result = read(fd, tmp_buf, read_len);
    if (0 >= result) {
        DEBUG_PRINTF("Read nvram result=%d\n", result);
        goto exit_point;
    }

    memset(sn, 0, *len);
    memcpy(sn, tmp_buf, result);
    *len = result;

    DEBUG_PRINTF("%s: sn=%s len=%d\n", __FUNCTION__, sn, result);
//    EAP_LOG_DEBUG("sn=%s len=%d\n", sn, result);

exit_point:
    if (0 <= fd) {
        close(fd);
    }
    return result;
}

int nvram_write_sn(uint8_t *sn, uint32_t len)
{
    int result = 0;
    char tmp_buf[EAP_SN_LENGTH];
    int write_len = MIN(len, EAP_SN_LENGTH);
    int fd = -1;
    struct stat st;

    if (0 >= len || EAP_SN_LENGTH < len) {
        DEBUG_PRINTF("Error parameter: len=%d\n", len);
        goto exit_point;
    }

    nvram_check_sn();

    result = stat(NVRAM_PATH, &st);
    if (0 == result && st.st_size != write_len)
    {
        truncate(NVRAM_PATH, write_len);
        DEBUG_PRINTF("Truncate nvram! write_len=%d\n", write_len);
    }

    fd = open(NVRAM_PATH, O_WRONLY);
    if (0 > fd) {
        DEBUG_PRINTF("Open nvram failed! fd=%d\n", fd);
        goto exit_point;
    }

    memset(tmp_buf, 0, sizeof(tmp_buf));
    memcpy(tmp_buf, sn, write_len);
    result = write(fd, tmp_buf, write_len);
    if (write_len != result) {
        DEBUG_PRINTF("Write nvram result=%d\n", result);
        goto exit_point;
    }

    DEBUG_PRINTF("%s: sn=%s len=%d\n", __FUNCTION__, sn, result);
//    EAP_LOG_DEBUG("sn=%s len=%d\n", sn, result);

exit_point:
    if (0 <= fd) {
        close(fd);
    }
    return result;
}

#if DEBUG_UNITTEST
void nvram_test_api()
{
    DEBUG_PRINTF("[%s:%d] FIX ME! Not impliment yet!\n", __FUNCTION__, __LINE__);
}
#endif

#ifdef __cplusplus
}
#endif
