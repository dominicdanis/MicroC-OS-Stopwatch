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

#define START_ADDR 0x00000000
#define END_ADDR 0x001FFFFF

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

static void appSetTimerCount(INT8C *numStart);
static INT8C * appGetTimerCount(void);


typedef enum {CLEAR,COUNT,HOLD} SW_STATE;
static SW_STATE TimeState;
static INT8C *appTimerCount;
static INT8C OutputTime[8] = "00:00.00";
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
    INT16U checksum;
    (void)p_arg;

    OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
    GpioDBugBitsInit();
    OSMutexCreate(&appTimerCountKey,
                  "Timer Count Mutex",
                  &os_err);
    TimeState = CLEAR;
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
    checksum = MemChkSum((INT8U *)START_ADDR, (INT8U *)END_ADDR);
    LcdDispHexWord(LCD_ROW_2,LCD_COL_1,LCD_LAYER_STARTUP,(const INT32U)checksum, 4);
    OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
* TASK HEADER
*****************************************************************************************/
static void appTimerControlTask(void *p_arg){

    OS_ERR os_err;
    INT8U kchar;
    INT8C *current;
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
                    SWCntrCntrlSet(1,0);                //start counting
                    break;
                case COUNT:
                    TimeState = HOLD;
                    SWCntrCntrlSet(0,0);                //hold counting
                    break;
                case HOLD:
                    TimeState = CLEAR;
                    SWCntrCntrlSet(0,1);                //clear counter
                    break;
                default:
                    TimeState = CLEAR;
                    SWCntrCntrlSet(1,0);                //clear counter
                    break;
            }
                break;
            case '#':
                current = appGetTimerCount();
                LcdDispString(LCD_ROW_2,LCD_COL_1,LCD_LAYER_TIMER,(INT8C *const)current);
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
    INT32U remain;
    INT32U out;
    (void)p_arg;

    while(1) {
        DB1_TURN_OFF();
        out = SWCountPend(0,&os_err);
        DB1_TURN_ON();
        remain = out%60000;
        out = (out-remain)/60000;
        OutputTime[0] = (INT8C)(out+48);
        out = remain;
        remain = out%6000;
        out = (out-remain)/6000;
        OutputTime[1] = (INT8C)(out+48);
        out = remain;
        remain = out%1000;
        out = (out-remain)/1000;
        OutputTime[3] = (INT8C)(out+48);
        out = remain;
        remain = out%100;
        out = (out-remain)/100;
        OutputTime[4] = (INT8C)(out+48);
        out = remain;
        remain = out%10;
        out = (out-remain)/10;
        OutputTime[6] = (INT8C)(out+48);
        OutputTime[7] = (INT8C)(remain+48);
        LcdDispString(LCD_ROW_1,LCD_COL_1,LCD_LAYER_TIMER,(INT8C *const)OutputTime);
        appSetTimerCount((INT8C *)OutputTime);
    }
}

/********************************************************************************/

static void appSetTimerCount(INT8C *current_time){
    OS_ERR os_err;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    appTimerCount = current_time;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
}

static INT8C * appGetTimerCount(){
    OS_ERR os_err;
    INT8C *current_time;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    current_time = appTimerCount;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
    return current_time;
}

