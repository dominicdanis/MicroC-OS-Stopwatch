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
* main()
*****************************************************************************************/
void main(void) {

    OS_ERR  os_err;

    K65TWR_BootClock();
    CPU_IntDis();               /* Disable all interrupts, OS will enable them  */

    OSInit(&os_err);                    /* Initialize uC/OS-III                         */
    OSTaskCreate(&appTaskStartTCB,                  /* Address of TCB assigned to task */
                 "Start Task",                      /* Name you want to give the task */
                 appStartTask,                      /* Address of the task itself */
                 (void *) 0,                        /* p_arg is not used so null ptr */
                 APP_CFG_TASK_START_PRIO,           /* Priority you assign to the task */
                 &appTaskStartStk[0],               /* Base address of task�s stack */
                 (APP_CFG_TASK_START_STK_SIZE/10u), /* Watermark limit for stack growth */
                 APP_CFG_TASK_START_STK_SIZE,       /* Stack size */
                 0,                                 /* Size of task message queue */
                 0,                                 /* Time quanta for round robin */
                 (void *) 0,                        /* Extension pointer is not used */
                 (OS_OPT_TASK_NONE), /* Options */
                 &os_err);                          /* Ptr to error code destination */
    OSStart(&os_err);               /*Start multitasking(i.e. give control to uC/OS)    */
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
    (void)p_arg;
    
    while(1){
    

    }
}

/*****************************************************************************************
* TASK HEADER
*****************************************************************************************/
static void appTimerDisplayTask(void *p_arg){

    OS_ERR os_err;
    (void)p_arg;

    while(1) {


    }
}

/********************************************************************************/
