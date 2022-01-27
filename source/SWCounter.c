/*****************************************************************************************
* SWCounter
* This module includes counting functionality for a basic stopwatch - swCounterTask.
* Takes control for counting state through mutex and writes present count state through
* simple synchronization buffer
*
* Last edit Dominic Danis 1/24/2022
*****************************************************************************************/
#include "SWCounter.h"
#include "MCUType.h"
#include "os.h"
#include "app_cfg.h"
#include "K65TWR_GPIO.h"
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB swCounterTaskTCB;
/*****************************************************************************************
* Allocate task stack
*****************************************************************************************/
static CPU_STK swCounterTaskStk[APP_CFG_SWCNT_TASK_STK_SIZE];
/*****************************************************************************************
* Allocate Mutex
*****************************************************************************************/
static OS_MUTEX swCntrCntrlKey;
/*****************************************************************************************
* Counting state
*****************************************************************************************/
typedef enum {COUNT,HOLD,ZERO}CNTRL_STATES;
static CNTRL_STATES swCntrCntrl = ZERO;
/*****************************************************************************************
* Buffer for synchronization
*****************************************************************************************/
typedef struct{
    INT32U count;
    OS_SEM flag;
}CNTBUFF_T;
static CNTBUFF_T swCntrBuffer;
/*****************************************************************************************
* Private function prototypes
*****************************************************************************************/
static CNTRL_STATES swCntrCntrlGet(void);
static void swCounterTask(void *p_arg);
/*****************************************************************************************
* SWCounterInit
* Initializes counter. Creates buffer and task
*****************************************************************************************/
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

/*****************************************************************************************
* swCounterTask
* Counter task for stopwatch. Updates buffer and signals that count has beeen updated. (Every 10ms)
* Dependent on shared resource swCntrCntrl
*****************************************************************************************/
static void swCounterTask(void *p_arg){
    OS_ERR os_err;
    CNTRL_STATES cntrl;
    CNTRL_STATES last_state;
    (void)p_arg;
    while(1){
        DB2_TURN_OFF();
        OSTimeDly(10,OS_OPT_TIME_PERIODIC,&os_err);
        DB2_TURN_ON();
        cntrl = swCntrCntrlGet();
        if(cntrl==COUNT){
            swCntrBuffer.count++;
            OSSemPost(&(swCntrBuffer.flag),OS_OPT_POST_ALL,&os_err);
        }
        else if(cntrl==ZERO && last_state!=cntrl){
            swCntrBuffer.count = 0;
            OSSemPost(&(swCntrBuffer.flag),OS_OPT_POST_ALL,&os_err);
        }
        else{}
        last_state = cntrl;                 //prevents rewriting when count is same
    }
}
/*****************************************************************************************
* SWCountPend
* Function for synchronization. When buffer flag is posted will return buffer data.
* Can use tout as timeout for pending. Passes pointer to error
*****************************************************************************************/
INT32U SWCountPend(INT16U tout, OS_ERR *os_err){
    OSSemPend(&(swCntrBuffer.flag),tout,OS_OPT_PEND_BLOCKING,(CPU_TS *)0, os_err);
    return swCntrBuffer.count;
}
/*****************************************************************************************
* SWCntrCntrlSet
* Setter for SWCntrCntrl. Sets state of counter dependent on enable and disable
* parameters passed
*****************************************************************************************/
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
    swCntrCntrl = temp_state;
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* swCntrCntrlGet
* Getter for SWCntrCntrl. Will return the state of the counter
*****************************************************************************************/
static CNTRL_STATES swCntrCntrlGet(){
    OS_ERR os_err;
    CNTRL_STATES state;
    OSMutexPend(&swCntrCntrlKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    state = swCntrCntrl;
    OSMutexPost(&swCntrCntrlKey, OS_OPT_POST_NONE, &os_err);
    return state;
}

