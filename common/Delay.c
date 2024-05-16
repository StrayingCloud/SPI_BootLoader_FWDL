/*
 * Delay.c
 *
 *  Created on: 2015-11-19
 *      Author: Administrator
 */


#include "Delay.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>
static SeDelayOperation tGlobalDelayOperation = {
			.fpSeDelayUs = SeSleepUs,
			.fpSeDelayNs = SeSleepNs};;

uint32_t SeDelayInit(SeDelayOperation tDelayOper)
{
	tGlobalDelayOperation = tDelayOper;
	return 1;
}

void SeDelayMs(uint32_t nMs)
{
	tGlobalDelayOperation.fpSeDelayUs(nMs * 1000);
}
void SeDelayUs(uint32_t nUs)
{
	tGlobalDelayOperation.fpSeDelayUs(nUs);
}

void SeDelayNs(uint32_t nNs)
{
	tGlobalDelayOperation.fpSeDelayNs(nNs);
}

void SeSleepMs(uint32_t nMs)
{
    int ret = 0;
    struct timespec tSleepTime, tRemTime;
    tSleepTime.tv_sec = nMs/1000;
    tSleepTime.tv_nsec = (nMs%1000) * 1000000;
    ret = nanosleep(&tSleepTime, &tRemTime);
    if(ret != 0){
        printf("clock awake ,remain time is %ld s %ld ns",tRemTime.tv_sec,tRemTime.tv_nsec);
    }
}

void SeSleepUs(uint32_t nUs)
{
    int ret = 0;
    struct timespec tSleepTime, tRemTime;
    tSleepTime.tv_sec = nUs/1000000;
    tSleepTime.tv_nsec = (nUs%1000000) * 1000;
    ret = nanosleep(&tSleepTime, &tRemTime);
    if(ret != 0){
        printf("clock awake ,remain time is %ld s %ld ns",tRemTime.tv_sec,tRemTime.tv_nsec);
    }
}

void SeSleepNs(uint32_t nNs)
{
    int ret = 0;
    struct timespec tSleepTime, tRemTime;
    tSleepTime.tv_sec = nNs/1000000000;
    tSleepTime.tv_nsec = nNs%1000000000;
    ret = nanosleep(&tSleepTime, &tRemTime);
    if(ret != 0){
        printf("clock awake ,remain time is %ld s %ld ns",tRemTime.tv_sec,tRemTime.tv_nsec);
    }
}
void SeDelayCycles(uint32_t nCycles)
{
	uint32_t iCount;
	for(iCount = 0; iCount < nCycles; iCount++);
}
