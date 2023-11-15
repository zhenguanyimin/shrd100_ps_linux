
#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum log_level
{
    LL_NONE = 0,
    LL_FATAL = 2,
    LL_ERROR,
    LL_WARNING,
    LL_NOTICE,
    LL_INFO,
    LL_DEBUG,
    LL_VERB,
    LL_ANY
} log_level_t;

typedef enum log_uart_chl
{
    LOG_PS_UART0 = 0,
    LOG_Pl_UART1 = 1,
    LOG_Pl_UART2 = 2,
	LOG_PS_TCP = 3,
	LOG_PS_USB = 4,
} log_uart_chl_t;


void log_set_lvl(log_level_t lvl);
log_level_t log_get_lvl_num(void);
const char *log_get_lvl_str(void);
void log_record(log_level_t lvl, const char *fmt, ...);
void log_dump(uint32_t base_addr, uint8_t *buf, uint32_t len);
uint8_t GetSwitchOutputChl( );
void SetSwitchOutputChl(uint8_t value);


#ifdef MODULE_NAME
#define lLOG_FATAL(format, args...) log_record(LL_FATAL, "%s: "             \
                                                         "[LOG-F] " format, \
                                              MODULE_NAME, ##args)
#define lLOG_ERROR(format, args...) log_record(LL_ERROR, "%s: "             \
                                                        "[LOG-E] " format, \
                                              MODULE_NAME, ##args)
#define lLOG_DEBUG(format, args...) log_record(LL_DEBUG, "%s: "             \
                                                        "[LOG-D] " format, \
                                              MODULE_NAME, ##args)
#define lLOG_INFO(format, args...) log_record(LL_INFO, "%s: "             \
                                                      "[LOG-I] " format, \
                                          MODULE_NAME,##args)
#define lLOG_VERBOSE(format, args...) log_record(LL_VERB, "%s: "             \
                                                         "[LOG-V] " format, \
                                                MODULE_NAME, ##args)
#endif
#ifndef __UNITTEST__
#define lLOG_FATAL(format, args...) log_record(LL_FATAL, "[LOG-F] " format, ##args)
#define lLOG_ERROR(format, args...) log_record(LL_ERROR, "[LOG-E] " format, ##args)
#define lLOG_DEBUG(format, args...) log_record(LL_DEBUG, "[LOG-D] " format, ##args)
#define lLOG_INFO(format, args...) log_record(LL_INFO, "[LOG-I] "  format, ##args)
#define lLOG_VERBOSE(format, args...) log_record(LL_VERB, "[LOG-V] " format, ##args)
#else
#define lLOG_FATAL(format, args...) 
#define lLOG_ERROR(format, args...) 
#define lLOG_DEBUG(format, args...) 
#define lLOG_INFO(format, args...) 
#define lLOG_VERBOSE(format, args...) 
#endif

#define lLOG_TRACE_POINT LOG_DEBUG("[%s:%d]\r\n", __FUNCTION__, __LINE__)

#ifdef __cplusplus
}
#endif


#endif /* LOG_H */
