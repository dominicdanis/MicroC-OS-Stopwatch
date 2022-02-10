/*****************************************************************************************
* AppLab2
* This is the main module for a stopwatch application. It includes a startup task with initializations,
* and tasks for implementing UI and stopwatch display on LCD.
*
* Last edit Dominic Danis 1/24/2022
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
#define TMIN_CONV 60000
#define MIN_CONV 6000
#define TSEC_CONV 1000
#define SEC_CONV 100
#define TMS_CONV 10
#define ASCII_OFFSET 48
#define MAX_TIME 359999
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTimerControlTaskTCB;
static OS_TCB appTimerDisplayTaskTCB;
/*****************************************************************************************
* Mutex Allocation
*****************************************************************************************/
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
/*****************************************************************************************
* Getter and setter for shared resource
*****************************************************************************************/
static void appSetTimerCount(INT8C *numStart);
static INT8C * appGetTimerCount(void);
/*****************************************************************************************
* UI states
*****************************************************************************************/
typedef enum {CLEAR,COUNT,HOLD} SW_STATE;
static SW_STATE appTimeState;
/*****************************************************************************************
* Shared data for appTimerCountKey Mutex
*****************************************************************************************/
static INT8C *appTimerCount; //>>>can not be pointer, no storage -1req
static INT8C appOutputTime[] = "00:00.00";
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
* This task runs once and is the deleted. Initializes modules, creates tasks and displays
* initial checksum
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
    LcdDispHexWord(LCD_ROW_2,LCD_COL_1,LCD_LAYER_STARTUP,(const INT32U)checksum, LCD_BYTE);
    appTimeState = CLEAR;
    OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
* appTimerControlTask
*
* Task implementing stopwatch UI. Pends on input from keypad and either sends control
* to counter module or displays current stopwatch count in LCD
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
            switch(appTimeState){
                case CLEAR:
                    appTimeState = COUNT;
                    SWCntrCntrlSet(1,0);                //start counting
                    break;
                case COUNT:
                    appTimeState = HOLD;
                    SWCntrCntrlSet(0,0);                //hold counting
                    break;
                case HOLD:
                    appTimeState = CLEAR;
                    SWCntrCntrlSet(0,1);                //clear counter
                    break;
                default:
                    appTimeState = CLEAR;
                    SWCntrCntrlSet(1,0);                //clear counter
                    break;
            }
                break;
            case '#':
                current = appGetTimerCount(); //>>>must have different layer -1req
                LcdDispString(LCD_ROW_2,LCD_COL_1,LCD_LAYER_TIMER,(INT8C *const)current);
                break;
            default:
                break;
        }
    }
}

/*****************************************************************************************
* appTimerDisplayTask
* Runs when count is updated  - from SWCounter module. Computes time based on count, stores in a
* shared resource and displays on LCD
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
        if(out>MAX_TIME){                                         //never count past 59:59.99
            out = MAX_TIME;
        }
        else{}
        remain = out%TMIN_CONV;                                   //compute and store time
        out = (out-remain)/TMIN_CONV;
        appOutputTime[0] = (INT8C)(out+ASCII_OFFSET);
        out = remain;
        remain = out%MIN_CONV;
        out = (out-remain)/MIN_CONV;
        appOutputTime[1] = (INT8C)(out+ASCII_OFFSET);
        out = remain;
        remain = out%TSEC_CONV;
        out = (out-remain)/TSEC_CONV;
        appOutputTime[3] = (INT8C)(out+ASCII_OFFSET);
        out = remain;
        remain = out%SEC_CONV;
        out = (out-remain)/SEC_CONV;
        appOutputTime[4] = (INT8C)(out+ASCII_OFFSET);
        out = remain;
        remain = out%TMS_CONV;
        out = (out-remain)/TMS_CONV;
        appOutputTime[6] = (INT8C)(out+ASCII_OFFSET);
        appOutputTime[7] = (INT8C)(remain+ASCII_OFFSET);
        LcdDispString(LCD_ROW_1,LCD_COL_1,LCD_LAYER_TIMER,(INT8C *const)appOutputTime);
        appSetTimerCount((INT8C *)appOutputTime);
    }
}

/*****************************************************************************************
* appSetTimerCount
* Setter for timerCount shared variable. Is accessed via mutex
*****************************************************************************************/
static void appSetTimerCount(INT8C *current_time){
    OS_ERR os_err;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    appTimerCount = current_time;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
}
/*****************************************************************************************
* appGetTimerCount
* Getter for timerCount shared variable. Is accessed via mutex
*****************************************************************************************/
static INT8C * appGetTimerCount(){
    OS_ERR os_err;
    INT8C *current_time;
    OSMutexPend(&appTimerCountKey, 0, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, &os_err);
    current_time = appTimerCount;
    OSMutexPost(&appTimerCountKey, OS_OPT_POST_NONE, &os_err);
    return current_time;
}

