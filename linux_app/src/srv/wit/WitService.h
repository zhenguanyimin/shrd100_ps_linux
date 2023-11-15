/** 
 * wireless image trasition service
 *
 * author: chenyili@autel.com
 *
 */

#ifndef WIT_SERVICE_H_
#define WIT_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif



typedef enum WIT_STATUS_e
{
	WIT_STATUS_ERROR = -2,       /* hardware fault */
	WIT_STATUS_DISABLE = -1,     /* wit was disable */
	WIT_STATUS_ENABLE_NO_CONNECT = 0,
	WIT_STATUS_ENABLE_CONNECTED = 1,
}WIT_STATUS_t;


extern int wit_service_start(const char *msg_prefix_name);
extern int wit_service_stop(void);
extern int wit_disable(void);
extern int wit_enable(void);
extern WIT_STATUS_t wit_status(void);



#ifdef __cplusplus
}
#endif


#endif


