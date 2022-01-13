/*****************************************************************************************
* A simple demo program for uCOS-III.
* It tests multitasking, the timer, and task semaphores.
* This version is written for the K65TWR board, LED8 and LED9.
* If uCOS is working the green LED should toggle every 100ms and the blue LED
* should toggle every 1 second.
* Version 2017.2
* 01/06/2017, Todd Morton
* Version 2018.1 First working version for MCUXpresso
* 12/06/2018 Todd Morton
* Version 2021.1 First working version for MCUX11.2
* 01/04/2021 Todd Morton
* Version 2022.1 First working version for MCUX11.4
* 01/02/2022 Todd Morton
* Error Traps Removed for Working Demo
* 1/09/2022 Dominic Danis
*****************************************************************************************/
#include "os.h"
#include "app_cfg.h"
#include "MCUType.h"
#include "K65TWR_ClkCfg.h"
#include "K65TWR_GPIO.h"
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTask1TCB;
static OS_TCB appTask2TCB;

/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK appTask1Stk[APP_CFG_TASK1_STK_SIZE];
static CPU_STK appTask2Stk[APP_CFG_TASK2_STK_SIZE];

/*****************************************************************************************
* Task Function Prototypes. 
*   - Private if in the same module as startup task. Otherwise public.
*****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appTask1(void *p_arg);
static void  appTask2(void *p_arg);

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

    (void)p_arg;                        /* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SYSTEM_CLOCK);
    GpioLED8Init();
    GpioLED9Init();
    GpioDBugBitsInit();

    OSTaskCreate(&appTask1TCB,                  /* Create Task 1                    */
                "App Task1 ",
                appTask1,
                (void *) 0,
                APP_CFG_TASK1_PRIO,
                &appTask1Stk[0],
                (APP_CFG_TASK1_STK_SIZE / 10u),
                APP_CFG_TASK1_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    OSTaskCreate(&appTask2TCB,    /* Create Task 2                    */
                "App Task2 ",
                appTask2,
                (void *) 0,
                APP_CFG_TASK2_PRIO,
                &appTask2Stk[0],
                (APP_CFG_TASK2_STK_SIZE / 10u),
                APP_CFG_TASK2_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    OSTaskDel((OS_TCB *)0, &os_err);
}

/*****************************************************************************************
* TASK #1
* Uses OSTimeDelay to signal the Task2 semaphore every second.
* It also toggles the green LED every 100ms.
*****************************************************************************************/
static void appTask1(void *p_arg){

    INT8U timcntr = 0;                              /* Counter for one second flag      */
    OS_ERR os_err;
    (void)p_arg;
    
    while(1){
    
        DB1_TURN_OFF();                             /* Turn off debug bit while waiting */
    	OSTimeDly(100,OS_OPT_TIME_PERIODIC,&os_err);     /* Task period = 100ms   */
        DB1_TURN_ON();                          /* Turn on debug bit while ready/running*/
        LED8_TOGGLE();                          /* Toggle green LED                     */
        timcntr++;
        if(timcntr == 10){                     /* Signal Task2 every second             */
            (void)OSTaskSemPost(&appTask2TCB,OS_OPT_POST_NONE,&os_err);
            timcntr = 0;
        }else{
        }
    }
}

/*****************************************************************************************
* TASK #2
* Pends on its semaphore and toggles the blue LED every second
*****************************************************************************************/
static void appTask2(void *p_arg){

    OS_ERR os_err;

    (void)p_arg;

    while(1) {                                  /* wait for Task 1 to signal semaphore  */

        DB2_TURN_OFF();                         /* Turn off debug bit while waiting     */
        OSTaskSemPend(0,                        /* No timeout                           */
                      OS_OPT_PEND_BLOCKING,     /* Block until posted                   */
                      (void *)0,                /* No timestamp                         */
                      &os_err);
        DB2_TURN_ON();                          /* Turn on debug bit while ready/running*/
        LED9_TOGGLE();;                         /* Toggle blue LED                    */
    }
}

/********************************************************************************/
