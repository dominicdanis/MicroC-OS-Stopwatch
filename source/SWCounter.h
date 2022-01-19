#include "MCUType.h"
#include "os.h"

#ifndef SWCNT_DEF
#define SWCNT_DEF

void SWCounterInit(void);
INT32U SWCountPend(INT16U tout, OS_ERR *os_err);
void SWCntrCntrlSet(INT8U enable, INT8U reset);

#endif
