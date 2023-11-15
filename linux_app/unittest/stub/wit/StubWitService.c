/** 
 * for ut stub
 *
 * author: jiawei@autel.com
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../src/srv/wit/WitService.h"

#ifdef __cplusplus
}
#endif

int wit_disable(void)
{
    return 0;
}

int wit_enable(void)
{
    return 0;
}

WIT_STATUS_t wit_status(void)
{
    WIT_STATUS_t result = WIT_STATUS_DISABLE;

    static int counter = 0;
    counter++;

    if ((counter > 5) && (counter <= 10))
    {
         result = WIT_STATUS_ENABLE_CONNECTED;
    }
    else if (counter > 10)
    {
         counter = 0;
         result = WIT_STATUS_DISABLE;
    }

    return result;
}

