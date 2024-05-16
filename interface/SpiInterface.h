#ifndef __SPIINTERFACE_H__
#define __SPIINTERFACE_H__

//-------------------------------------------------------include---------------------------------------------------------
#include "DbgrCommon.h"
//-------------------------------------------------------macro---------------------------------------------------------

class SpiInterface
{
public:
	SpiInterface(char *pName,
				 std::shared_ptr<spdlog::logger> logger=spdlog::default_logger());
	~SpiInterface();

	DebuggerExecutingState_t SetFrequency(uint32_t nFreqencyHz);
	DebuggerExecutingState_t ReadFrequency(uint32_t &qFreqencyHz);
	DebuggerExecutingState_t SwitchSpiMode(SpiCpolnCphaMode_t tSpiModeMask);
	DebuggerExecutingState_t sTransmit(uint8_t anWriteList[],uint8_t anReadList[],uint32_t nTransmitLength);
	DebuggerExecutingState_t ModuleEnable(void) ;
	DebuggerExecutingState_t ModuleDisable(void);

public:
	std::shared_ptr<spdlog::logger> logger;
};


//-------------------------------------------------------function---------------------------------------------------------




#endif
