#include "CmdprocBase.h"

enum {
    FLAG_BOTH_DISCONNECT = 0,
    FLAG_TCP_CONNECT = 1,   // 包括有线,无线和图传的TCP连接
    FLAG_TYPEC_CONNECT = 2,
    FLAG_BOTH_CONNECT = 3,
};

class HeartBeatCmdproc : public CmdprocBase
{
public:
    HeartBeatCmdproc();
    ~HeartBeatCmdproc();
    void ShowInfo(uint8_t level)override;
    bool _GetHeartBeatFlag(link_c2_t link_type){return m_c2_heartbeat_flag[link_type];}
protected:
    void _OnTimer(uint8_t timerType) override;
    void _OnTargetWarning(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnRefreshWorkStatus(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnRefreshFaultStatus(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnSetCovertMode(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnShowHeartBeatModuleCmd(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnUpdateC2HeartBeatTime(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _OnUpdateALinkTime(const Cmdproc_Data& inMsg, Cmdproc_Data& outMsg);
    void _RegisterCmd() override;
    void _ReportHeartBeat();
    bool _CheckLinkStatus();
    int _TCPC2TimeOutCheck();
    void _TypecSetRevHeartBeatFlag(bool flag);
    void _LedLinkStatus(bool isLinked);
    void _RequestC2TimeInfo();

private:
    bool _isWitLinked;
    bool _isWifiLinked;
    std::uint16_t _usbLinkedCnt;
    uint64_t m_c2_heartbeat_tick[MAX_LINK_C2_NUM];
    uint64_t m_c2_tcp_tick[MAX_LINK_C2_NUM];
    bool m_c2_heartbeat_flag[MAX_LINK_C2_NUM];
	bool hearbeat_sync_typec_flag ;
	int _c2HeartBeatStatus;	//0-both disconnect, 1-tcp connect, 2-typec connect, 3-both connect
};
