#ifndef __DBGRCOMMON_H__
#define __DBGRCOMMON_H__

//------------------------------------include----------------------------
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <exception>
#include <stdint.h>
#include "spdlog/spdlog.h"
#include "Delay.h"


//------------------------------------spi mode------------------------------
typedef enum{
	SpiCpol0nCpha0Mode = 0,
	SpiCpol0nCpha1Mode = 1,
	SpiCpol1nCpha0Mode = 2,
	SpiCpol1nCpha1Mode = 3,
}SpiCpolnCphaMode_t;

//------------------------------------macro------------------------------

#define DbgrDelayOperation									SeDelayOperation
#define DbgrInitialDelayInstance(tDelayOperation)			SeDelayInit(tDelayOperation)
#define DbgrDelayUs(nUs)									SeDelayUs(nUs)//while(0){}
#define DbgrDelayMs(nMs)									SeDelayMs(nMs)//while(0){}
#define DbgrDelayNs(nNs)									SeDelayNs(nNs)//while(0){}

typedef enum{
	DebuggerExecutingNormal = 1,											//executing normal not error
	DebuggerExecutingDriverFault	= -1,									//executing fault in driver response
	DebuggerExecutingDriverBusy	= -2,										//executing busy in driver response
	DebuggerExecutingProtocolBusy = -3,									//executing busy in protocol response
	DebuggerExecutingProtocolFault = -4,									//executing Fault in protocol response
	DebuggerExecutingHardwareFault = -5,									//executing Fault in hardware response
	DebuggerExecutingHardwareBusy = -6,									//executing busy in hardware mutex
	DebuggerExecutingDutOperationTimeout = -7,					//waiting timeout in operating DUT
	DebuggerExecutingDutIllegalOperation = -8,						//DUT illegal operation occurs
	DebuggerExecutingSoftParamFault = -9,								//function param input error

	DebuggerExecutingFirmwareNotMatch = -16,								//firmware is not match or firmware's length out of length
	DebuggerExecutingFileOperationError= -17,						//firmware File Operation error
	DebuggerExecutingFirmwareFileNotSuitable = -18,			//file is not exist or not suitable
	DebuggerExecutingConfigureFileNotSuitable = -19,         //configure file is not exist or not suitable
	DebuggerExecutingParamFault = -30,									//inner operation param error
	DebuggerExecutingSoftTimeOut = -31,                             //timeout in software inner executing
	DebuggerExecutingSoftError = -32,                                    //error in software inner executing

	DebuggerExecutingProgrammingCheckoutError = -100,	//fwdl checkout error
	DebuggerExecutingProgrammingEraseError = -101,      //fwdl erase checkout error
	DebuggerExecutingFunctionInexistentError = -110,	//function is not exist		
	DebuggerExecutingRpcServerError = -111,		//RPC server error
	DebuggerExecutingWaitingTomeout = -112,     //wait remote server time out
	DebuggerExecutingUndefinedFault = -127,							//undefine error occurs

	DebuggerExecutingNullPointerParamFault = -1000,
}DebuggerExecutingState_t;


std::string DbgrErrorState(DebuggerExecutingState_t tDebuggerState);

#endif

