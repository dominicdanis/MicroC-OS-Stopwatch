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
/*****************************************************************************************
* SWCounterInit
* Initializes counter. Creates buffer and task
*****************************************************************************************/
void SWCounterInit(void);
/*****************************************************************************************
* SWCountPend
* Function for synchronization. When buffer flag is posted will return buffer data.
* Can use tout as timeout for pending. Passes pointer to error
*****************************************************************************************/
INT32U SWCountPend(INT16U tout, OS_ERR *os_err);
/*****************************************************************************************
* SWCntrCntrlSet
* Setter for SWCntrCntrl. Sets state of counter dependent on enable and disable
* parameters passed
*****************************************************************************************/
void SWCntrCntrlSet(INT8U enable, INT8U reset);

#endif
