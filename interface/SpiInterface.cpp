
#include "SpiInterface.h"
#include <iostream>


SpiInterface::SpiInterface(char *pName, std::shared_ptr<spdlog::logger> logger)
{
	this->logger = logger;
	this->logger->debug("SpiInterface create with dev name {}", pName);
}

SpiInterface::~SpiInterface(){}


DebuggerExecutingState_t SpiInterface::SetFrequency(uint32_t nFreqencyHz)
{
	this->logger->debug("SPI SetFrequency={:d}", nFreqencyHz);
	return DebuggerExecutingNormal;
}


DebuggerExecutingState_t SpiInterface::SwitchSpiMode(SpiCpolnCphaMode_t tSpiModeMask)
{
	this->logger->debug("SPI SwitchSpiMode to {:d}", int(tSpiModeMask));
	return DebuggerExecutingNormal;
}


DebuggerExecutingState_t SpiInterface::sTransmit(uint8_t anWriteList[],uint8_t anReadList[],uint32_t nTransmitLength)
{
	this->logger->debug("SPI transmit data");
	this->logger->debug("SPI CS=1");
	this->logger->debug("SPI CS=0");
	this->logger->debug("SPI write and read.");
	this->logger->debug("SPI CS=1");

	return DebuggerExecutingNormal;
}


DebuggerExecutingState_t SpiInterface::ModuleEnable(void)
{
	this->logger->debug("SPI module enable");
	return DebuggerExecutingNormal;
}


DebuggerExecutingState_t SpiInterface::ModuleDisable(void)
{
	this->logger->debug("SPI module disable");
	return DebuggerExecutingNormal;
}
