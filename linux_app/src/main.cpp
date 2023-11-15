
#include "TracerMainSys.h"
#include "eap_pub.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <stdarg.h>
//#include <syslog.h>
#include "ThreadBase.h"
#include "eap_os_adapter.h"
#include "../srv/wit/WitService.h"
#include "../srv/bluetooth/bt_gatt.h"

#if 1
std::shared_ptr<TracerMainSys> sys = nullptr;
#else
TracerMainSys sys(DEV_RADAR, 0);
#endif


extern "C"
{
    void init_debug_telnet();
    void trace_exception_init();
}

SysBase& EapGetSys()
{
  return *sys;
}


int main(int argc,char*argv[])
{
    // 解析命令行参数, 支持打印版本信息
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-V" || arg == "--version") {
            std::cout << "Skyfend Technology Co., Ltd" << std::endl;
            std::cout << "linux_app.elf version: " << APP_VERSION << std::endl;         // 输出版本信息
            std::cout << "build time: " << __DATE__ << "  " << __TIME__ << std::endl;   // 输出编译时间
            return 0;
        }
    }

    eap_set_mqueue_attr(10000,8192);
    log_set_lvl(LL_ERROR);
    sys = std::shared_ptr<TracerMainSys>(new TracerMainSys(DEV_TRACER, 0));
    //eap_log(LOG_INFO,"this is TracerMainSys %s:%s:%d","arm","main.cpp",12345);
    //EAP_LOG_INFO("this is EAP_LOG_INFO TracerMainSys %s:%s:%d","arm","main.cpp",12345);
    //EAP_LOG_ERROR("this is EAP_LOG_ERROR TracerMainSys %s:%s:%d","arm","main.cpp",12345);
    //EAP_LOG_DEBUG("this is EAP_LOG_DEBUG TracerMainSys %s:%s:%d","arm","main.cpp",12345);
    //EAP_LOG_VERBOSE("this is EAP_LOG_VERBOSE TracerMainSys %s:%s:%d","arm","main.cpp",12345);
    if(sys != nullptr)
    {
        sys->Init();
    }

    init_debug_telnet();
    //trace_exception_init();
    wit_service_start("witd");
	bluetooth_init();
    while(1) usleep(1000000);

    return 0 ;
}
