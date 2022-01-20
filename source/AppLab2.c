/*****************************************************************************************
* HEADER WILL GO HERE
*****************************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "K65TWR_ClkCfg.h"
#include "K65TWR_GPIO.h"
#include "MemoryTools.h"
#include "uCOSKey.h"
#include "LcdLayered.h"
#include "SWCounter.h"
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTimerControlTaskTCB;
static OS_TCB appTimerDisplayTaskTCB;

static OS_MUTEX appTimerCountKey;

/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK appTimerControlTaskStk[APP_CFG_TIMER_CTRL_STK_SIZE];
static CPU_STK appTimerDisplayTaskStk[APP_CFG_TIMER_DISP_STK_SIZE];

/*****************************************************************************************
* Task Function Prototypes. 
*****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appTimerControlTask(void *p_arg);
static void  appTimerDisplayTask(void *p_arg);

static void appSetTimerCount(INT32U timer_val);
static INT32U appGetTimerCount();


typedef enum {CLEAR,COUNT,HOLD} SW_STATE;
static SW_STATE TimeState;
static INT32U appTimerCount;
/*****************************************************************************************
* main()
*****************************************************************************************/
void main(void) {

    OS_ERR  os_err;

    K65TWR_BootClock();
    CPU_IntDis();

    OSInit(&os_err);
    OSTaskCreate(&appTaskStartTCB,
                 "Start Task",
                 appStartTask,
                 (void *) 0,
                 APP_CFG_TASK_START_PRIO,
                 &appTaskStartStk[0],
                 (APP_CFG_TASK_START_STK_SIZE/10u),
                 APP_CFG_TASK_START_STK_SIZE,
                 0,
                 0,
                 (void *) 0,
                 (OS_OPT_TASK_NONE),
                 &os_err);
    OSStart(&os_err);
}

/*****************************************************************************************
* STARTUP TASK
* This should run once and be deleted. Could restart everything by creating.
*****************************************************************************************/
static void appStartTask(void *p_arg) {

    OS_ERR os_err;

    (void)p_arg;

    OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
    GpioDBugBitsInit();

    OSMutexCreate(&appTimerCountKey,
                  "Timer Count Mutex",
                  &os_err);

    //DISPLAY THE CHECKSUM SOMEWHERE IN HERE

    OSTaskCreate(&appTimerControlTaskTCB,
                "appTimerControl ",
                appTimerControlTask,
                (void *) 0,
                APP_CFG_TIMER_CTRL_PRIO,
                &appTimerControlTaskStk[0],
                (APP_CFG_TIMER_CTRL_STK_SIZE / 10u),
                APP_CFG_TIMER_CTRL_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    OSTaskCreate(&appTimerDisplayTaskTCB,
                "appTimerDisplay ",
                appTimerDisplayTask,
                (void *) 0,
                APP_CFG_TIMER_DISP_PRIO,
                &appTimerDisplayTaskStk[0],
                (APP_CFG_TIMER_DISP_STK_SIZE / 10u),
                APP_CFG_TIMER_DISP_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    SWCounterInit();
    KeyInit();
    LcdInit();
    OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
* TASK HEADER
*****************************************************************************************/
static void appTimerControlTask(void *p_arg){

    OS_ERR os_err;
    INT8U kchar;
    (void)p_arg;
    
    while(1){
        DB0_TURN_OFF();
        kchar = KeyPend(0,&os_err);
        DB0_TURN_ON();
        switch (kchar){
            case '*':
            switch(TimeState){
                case CLEAR:
                    TimeState = COUNT;
                    //Tell counter module to increment every 10ms
                    break;
                case COUNT:
                    TimeState = HOLD;
                    //Tell counter module to stop counting
                    break;
                case HOLD:
                    TimeState = CLEAR;
                    //Tell counter module to stop counting (should already be doing that)
                    break;
                default:
                    TimeState = CLEAR;
                    //Tell counter module to stop counting
                    break;
            }
                break;
            case '#':
                //check if the display already has something
                //if it doesn't put the current display there
                //if it does, clear
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************************
* TASK HEADER
*****************************************************************************************/
static void appTimerDisplayTask(void *p_arg){

    OS_ERR os_err;
    INT32U dispNums;
    (void)p_arg;

    while(1) {
        DB1_TURN_OFF();
        //dispNums = SWCountPend();
        DB1_TURN_ON();
        //Figure out how to display - what are we recieving in that 32 bit int?

    }
}

/********************************************************************************/

static void appSetTimerCount(INT32U timer_val){
    OS_ERR os_err;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    appTimerCount = timer_val;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
}

static INT32U appGetTimerCount(){
    OS_ERR os_err;
    INT32U timer_val;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    timer_val = appTimerCount;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
    return timer_val;
}
