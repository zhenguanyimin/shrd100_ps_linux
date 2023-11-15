#include "../../../eap/include/thread/ThreadBase.h"

#define CFG_NO_USING_RID_CAPTURE 1

#define IFNAME_REMOTEID "/tmp/ifname_remoteid"

class RemoteIdWifiThread : public ThreadBase
{
    DECLARE_THREAD(RemoteIdWifiThread)
   // DECLARE_TIMER1()
public:
    bool Run() override;

	static int remoteIdSwitchChannel(uint32_t chn);
	
protected:
    void _InitQueue(uint16_t& num) override;
    // void _RegisterTimer() override;
    void _OnCommand(const Thread_Msg& msg) override;
    int _GetDeviceType(uint8_t* package_payload);
private:
};

