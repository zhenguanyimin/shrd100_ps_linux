#include <string>

#include "CliThread.h"
#include "SysBase.h"
#include "FreeRTOS_CLI.h"
#include "HalBase.h"

DEFINE_THREAD(CliThread, CLI)

extern SysBase& EapGetSys();

void CliThread::_InitQueue(uint16_t& num)
{
    num = 10;
}

void CliThread::_OnCommand(const Thread_Msg& msg)
{
    memset(_outputString, 0, sizeof(_outputString));
    FreeRTOS_CLIProcessCommand((char*)msg.buf, _outputString, sizeof(_outputString));
    HalMgr* halMgr = EapGetSys().GetHalMgr();
    if (nullptr != halMgr)
    {
        Hal_Data inData;
        inData.length = strlen((const char *)_outputString);
        inData.buf = (uint8_t*)_outputString;
        if (EAP_SUCCESS != halMgr->OnSet(EAP_DRVCODE_CLI_OUT, inData))
        {
            EAP_LOG_ERROR("Hal Set Error(drvcode:%d)", EAP_DRVCODE_CLI_OUT);
        }
    }
}

void CliThread::ShowRegisteredCmds()
{
    const CLI_Definition_List_Item_t *pxCommand = GetCLIRegisteredCommands();
    int total = 0;
    size_t xCommandStringLengthMax = 0;
    for (; pxCommand != NULL; pxCommand = pxCommand->pxNext )
    {
        ++total;
        const char *pcRegisteredCommandString = pxCommand->pxCommandLineDefinition->pcCommand;
        size_t xCommandStringLength = strlen( pcRegisteredCommandString );
        if (xCommandStringLength > xCommandStringLengthMax) xCommandStringLengthMax = xCommandStringLength;
    }
    cout << hex;
    cout << "\n" << "CliCmdsNum: " << total;
    cout << "\n" << "CliCmds: ";
    cout << "\n" << "  No.  | CliCmd                      | ParamsNum | Func         | Help";
    int i = 0;
    for (pxCommand = GetCLIRegisteredCommands(); pxCommand != NULL; pxCommand = pxCommand->pxNext, ++i )
    {
        const char *pcRegisteredCommandString = pxCommand->pxCommandLineDefinition->pcCommand;
        size_t xCommandStringLength = strlen( pcRegisteredCommandString );
        string help(pxCommand->pxCommandLineDefinition->pcHelpString);
        size_t pos = 0;
        while ((pos = help.find("\r\n", pos)) != string::npos) {
            help.replace(pos, 2, " ");
            pos += 1;
        }
        cout << "\n  " << right << setw(4) << i;
        cout << " | " << left << setw(xCommandStringLengthMax) << pcRegisteredCommandString;
        cout << " | " << left << setw(9) << (int16_t)pxCommand->pxCommandLineDefinition->cExpectedNumberOfParameters;
        cout << " | " << left << setw(12) << (uint64_t)pxCommand->pxCommandLineDefinition->pxCommandInterpreter;
        cout << " | " << left << help;
    }
}

extern "C"
{
    void ShowCliRegisteredCmds()
    {
        CliThread::ShowRegisteredCmds();
    }
}
