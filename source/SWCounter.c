#include "SWCounter.h"
#include "MCUType.h"
#include "os.h"
#include "app_cfg.h"
#include "K65TWR_GPIO.h"


static OS_TCB swCounterTaskTCB;
static CPU_STK swCounterTaskStk[APP_CFG_SWCNT_TASK_STK_SIZE];
static OS_MUTEX swCntrCntrlKey;
typedef enum {COUNT,HOLD,ZERO}CNTRL_STATES;
typedef struct{
    INT32U count;
    OS_SEM flag;
}CNTBUFF_T;

static CNTBUFF_T swCntrBuffer;
static CNTRL_STATES SWCntrCntrl = ZERO;

static CNTRL_STATES SWCntrCntrlGet(void);
static void swCounterTask(void *p_arg);


void SWCounterInit(void){

    OS_ERR os_err;

    OSSemCreate(&(swCntrBuffer.flag),"SWCounter flag", 0, &os_err);
    swCntrBuffer.count = 0;
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
    CNTRL_STATES last_state;

    (void)p_arg;

    while(1){
        DB2_TURN_OFF();
        OSTimeDly(10,OS_OPT_TIME_PERIODIC,&os_err);
        while(os_err != OS_ERR_NONE){
        }
        DB2_TURN_ON();
        cntrl = SWCntrCntrlGet();
        if(cntrl==COUNT){
            swCntrBuffer.count++;
            OSSemPost(&(swCntrBuffer.flag),OS_OPT_POST_ALL,&os_err);
        }
        else if(cntrl==ZERO && last_state!=cntrl){
            swCntrBuffer.count = 0;
            OSSemPost(&(swCntrBuffer.flag),OS_OPT_POST_ALL,&os_err);
        }
        else{}
        last_state = cntrl;
    }
}



INT32U SWCountPend(INT16U tout, OS_ERR *os_err){
    OSSemPend(&(swCntrBuffer.flag),tout,OS_OPT_PEND_BLOCKING,(CPU_TS *)0, os_err);
    return swCntrBuffer.count;
}



//Consider a way to check out the key for shorter amount of time
void SWCntrCntrlSet(INT8U enable, INT8U reset){
    OS_ERR os_err;
    CNTRL_STATES temp_state;
    if(reset==1){
            temp_state = ZERO;
        }
        else if(enable==1 && reset ==0){
            temp_state = COUNT;
        }
        else if(enable==0 && reset==0){
            temp_state = HOLD;
        }
        else{}
    OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    SWCntrCntrl = temp_state;
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
}

static CNTRL_STATES SWCntrCntrlGet(){
    OS_ERR os_err;
    CNTRL_STATES state;
    OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    state = SWCntrCntrl;
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
    return state;
}

