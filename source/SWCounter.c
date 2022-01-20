#include "SWCounter.h"
#include "MCUType.h"
#include "os.h"
#include "app_cfg.h"
#include "K65TWR_GPIO.h"


static OS_TCB swCounterTaskTCB;
static CPU_STK swCounterTaskStk[APP_CFG_SWCNT_TASK_STK_SIZE];
static OS_MUTEX swCntrCntrlKey;
typedef enum {COUNT,HOLD}CNTRL_STATES;
static CNTRL_STATES SWCntrCntrl;
static CNTRL_STATES SWCntrCntrlGet();

static void swCounterTask(void *p_arg);




void SWCounterInit(void){

    OS_ERR os_err;

    OSMutexCreate(&swCntrCntrlKey,"sw control mutex", &os_err);

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


static void swCounterTask(void *p_arg){

    OS_ERR os_err;
    CNTRL_STATES cntrl;

    (void)p_arg;

    while(1){
        DB2_TURN_OFF();
        OSTimeDly(10,OS_OPT_TIME_PERIODIC,&os_err);
        while(os_err != OS_ERR_NONE){
        }
        DB2_TURN_ON();
        cntrl = SWCntrCntrlGet();
        if(cntrl==COUNT){

        }
        else if(cntrl==HOLD){

        }
        else{}
    }
}



INT32U SWCountPend(INT16U tout, OS_ERR *os_err){


}
void SWCntrCntrlSet(INT8U enable, INT8U reset){
    OS_ERR os_err;
    OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    if(reset==1){

    }
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
}

static CNTRL_STATES SWCntrCntrlGet(){
    OS_ERR os_err;
    OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
}

