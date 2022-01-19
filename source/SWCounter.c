#include "SWCounter.h"
#include "MCUType.h"
#include "os.h"
#include "app_cfg.h"
#include "K65TWR_GPIO.h"


static OS_TCB swCounterTaskTCB;

static CPU_STK swCounterTaskStk[APP_CFG_SWCNT_TASK_STK_SIZE];

static void swCounterTask(void *p_arg);

static void swCounterTask(void *p_arg){

    OS_ERR os_err;

    (void)p_arg;

    while(1){
        DB2_TURN_OFF();
        OSTimeDly(10,OS_OPT_TIME_PERIODIC,&os_err);
        while(os_err != OS_ERR_NONE){

        }
        DB2_TURN_ON();
    }
}


void SWCounterInit(void){

    OS_ERR os_err;

    OSTaskCreate(&swCounterTaskTCB,
                 "swCntTask",
                 swCounterTask,
                 (void *)0,
                 APP_CFG_SWCNT_TASK_PRIO,
                 &swCounterTaskStk[0],
                 APP_CFG_SWCNT_TASK_STK_SIZE/10,
                 APP_CFG_SWCNT_TASK_STK_SIZE,
                 0,
                 0,
                 (void *)0,
                 OS_OPT_TASK_NONE,
                 &os_err);
    while(os_err != OS_ERR_NONE){

    }

}



INT32U SWCountPend(INT16U tout, OS_ERR *os_err){


}
void SWCntrCntrlSet(INT8U enable, INT8U reset){


}
