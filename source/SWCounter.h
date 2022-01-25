/*****************************************************************************************
* SWCounter
* This module includes counting functionality for a basic stopwatch - swCounterTask.
* Takes control for counting state through mutex and writes present count state through
* simple synchronization buffer
*
* Last edit Dominic Danis 1/24/2022
*****************************************************************************************/
#include "MCUType.h"
#include "os.h"

#ifndef SWCNT_DEF
#define SWCNT_DEF

void SWCounterInit(void);
INT32U SWCountPend(INT16U tout, OS_ERR *os_err);
void SWCntrCntrlSet(INT8U enable, INT8U reset);

#endif
