/* Delay.h
 *
 *  Created on: 2015-11-19
 *      Author: Administrator
 */

#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * define platform delay method
 */
typedef struct
{
    void (*fpSeDelayUs)(uint32_t nUs); 
    void (*fpSeDelayNs)(uint32_t nNs); 
}SeDelayOperation;

/*
 * systick or RTC config for delay method
 */
uint32_t  SeDelayInit(SeDelayOperation tDelayOper);
void SeDelayNs(uint32_t nNs);
/*
 * delay microsecond
 */
void SeDelayUs(uint32_t nUs);

/*
 * delay Milliseconds
 */
void SeDelayMs(uint32_t nMs);

/*
 * sleep Milliseconds
 */
void SeSleepMs(uint32_t nMs);

/*
 * sleep Microseconds
 */
void SeSleepUs(uint32_t nUs);
void SeSleepNs(uint32_t nNs);
/*
 * delay for some non-operation instructs
 */
void SeDelayCycles(uint32_t nCycles);

#ifdef __cplusplus
}
#endif

#endif /* _DELAY_H_ */
