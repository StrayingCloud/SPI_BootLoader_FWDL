

#include "DbgrCommon.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
// #include <execinfo.h> //must complie with  -rdynamic


std::string DbgrErrorState(DebuggerExecutingState_t tDebuggerState){
    switch((int)tDebuggerState){
        case DebuggerExecutingNormal:
            return "executing normal";
        case DebuggerExecutingDriverFault:
            return "executing fault in driver response";
        case DebuggerExecutingDriverBusy:
            return "executing busy in driver response";
        case DebuggerExecutingProtocolBusy:
            return "executing busy in protocol response";
        case DebuggerExecutingProtocolFault:
            return "executing Fault in protocol response";
        case DebuggerExecutingHardwareFault:
            return "executing Fault in hardware response";
        case DebuggerExecutingHardwareBusy:
            return "executing busy in hardware mutex";
        case DebuggerExecutingDutOperationTimeout:
            return "waiting timeout in operating DUT";
        case DebuggerExecutingDutIllegalOperation:
            return "DUT illegal operation occurs";
        case DebuggerExecutingSoftParamFault:
            return "function param input error";
        case DebuggerExecutingSoftTimeOut:
            return "timeout in software inner executing";
        case DebuggerExecutingSoftError:
            return "error in software inner executing";
        case DebuggerExecutingFirmwareNotMatch:
            return "firmware is not match or firmware's length out of length";
        case DebuggerExecutingFileOperationError:
            return "firmware File Operation error";
        case DebuggerExecutingFirmwareFileNotSuitable:
            return "file is not exist or not suitable";
        case DebuggerExecutingParamFault:
            return "inner operation param error";
        case DebuggerExecutingProgrammingCheckoutError:
            return "fwdl checkout error";
        case DebuggerExecutingProgrammingEraseError:
            return "fwdl erase checkout error";
        case DebuggerExecutingUndefinedFault:
            return "undefine error occurs";
        case DebuggerExecutingConfigureFileNotSuitable :
            return "configure file is not exist or not suitable";
        case DebuggerExecutingFunctionInexistentError:
            return "do not support this function";
        case DebuggerExecutingRpcServerError:
            return "rpc response error";
        case DebuggerExecutingWaitingTomeout:
            return "wait remote server time out";
        case DebuggerExecutingNullPointerParamFault:
            return "NULL pointer param";
        default:
            break;
    }
    return "undefine error occurs";
}


// #endif

