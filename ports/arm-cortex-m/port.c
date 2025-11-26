// Cortex-M3 port glue: first-task startup and context switch handlers.
#include "port.h"

void vPortSVCHandler(void);
void vPortPendSVHandler(void);

void SVC_Handler(void) __attribute__((alias("vPortSVCHandler")));
void PendSV_Handler(void) __attribute__((alias("vPortPendSVHandler")));
void SysTick_Handler(void) __attribute__((alias("xPortSysTickHandler")));


static void prvStartFirstTask(void);
static void prvTaskExitError(void);
static void prvResetNextTaskUnblockTime(void);

#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS 4
#endif

static inline uint32_t prvCalcBASEPRI(void)
{
    /* Align BASEPRI to implemented priority bits (e.g., 4 bits on Cortex-M4). */
    const uint32_t shift = (uint32_t)(8U - __NVIC_PRIO_BITS);
    return (configKERNEL_INTERRUPT_PRIORITY << shift) & 0xFFU;
}



static void prvTaskExitError(void)
{
    for (;;)
    {
        // should never return
    }
}

uint32_t *rtosPortInitialiseStack(uint32_t *pxTopOfStack,
                                  TaskFunction_t pxCode,
                                  void *pvParameters)
{
    pxTopOfStack--;
    *pxTopOfStack = portINITIAL_XPSR;

    pxTopOfStack--;
    *pxTopOfStack = ((uint32_t)pxCode) & portSTART_ADDRESS_MASK; // PC

    pxTopOfStack--;
    *pxTopOfStack = (uint32_t)prvTaskExitError; // LR

    pxTopOfStack -= 5;                          // R12, R3, R2, R1
    *pxTopOfStack = (uint32_t)pvParameters;     // R0
    pxTopOfStack -= 8;                          // R11-R4

    return pxTopOfStack;
}

void vPortSetupTimerInterrupt(void)
{
    portNVIC_SYSTICK_LOAD_REG = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;

    portNVIC_SYSTICK_CTRL_REG = (   portNVIC_SYSTICK_CLK_BIT    |
                                    portNVIC_SYSTICK_INT_BIT    |
                                    portNVIC_SYSTICK_ENABLE_BIT);
}


void xPortStartScheduler(void)
{
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;

    vPortSetupTimerInterrupt();

    prvStartFirstTask();
}

void vTaskSwitchContext(void)
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
#if 0
    if(pxCurrentTCB == &Task_IDE_TCB)
    {
        if(Task1_TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB =&Task1_TCB;
        }
        else if(Task2_TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB =&Task2_TCB;
        }
        else
        {
            return;
        }
    }
    else
    {
        if(pxCurrentTCB == &Task1_TCB)
        {
            if(Task2_TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task2_TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &Task_IDE_TCB;
            }
            else
            {
                return;
            }
        }
        else if(pxCurrentTCB == &Task2_TCB)
        {
            if(Task1_TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task1_TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task_IDE_TCB;
            }
            else
            {
                return;
            }
        }
    }

#endif



#if 0
    if (pxCurrentTCB == &Task1_TCB)
    {
        pxCurrentTCB = &Task2_TCB;
    }
    else if (pxCurrentTCB == &Task2_TCB)
    {
        pxCurrentTCB = &Task1_TCB;
    }
    else
    {
        pxCurrentTCB = &Task1_TCB;
    }
#endif
}

__attribute__((naked)) static void prvStartFirstTask(void)
{
    __asm__ volatile("ldr r0, =0xE000ED08");  // VTOR
    __asm__ volatile("ldr r0, [r0]");
    __asm__ volatile("ldr r0, [r0]");
    __asm__ volatile("msr msp, r0");          // set MSP
    __asm__ volatile("cpsie i");
    __asm__ volatile("cpsie f");
    __asm__ volatile("dsb");
    __asm__ volatile("isb");
    __asm__ volatile("svc 0");                // enter SVC to start first task
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("bx lr");
}

extern TCB_t *pxCurrentTCB;

__attribute__((naked)) void vPortSVCHandler(void)
{
    __asm__ volatile("hdr_svc:");
    __asm__ volatile("ldr r3, =pxCurrentTCB");
    __asm__ volatile("ldr r1, [r3]");
    __asm__ volatile("ldr r0, [r1]");         // top of stack
    __asm__ volatile("ldmia r0!, {r4-r11}");
    __asm__ volatile("msr psp, r0");
    __asm__ volatile("isb");
    __asm__ volatile("mov r0, #0");
    __asm__ volatile("msr basepri, r0");
    __asm__ volatile("orr r14, #0xd");
    __asm__ volatile("bx r14");
}

__attribute__((naked)) void vPortPendSVHandler(void)
{
    __asm__ volatile("hdr_pendsv:");
    __asm__ volatile("mrs r0, psp");
    __asm__ volatile("isb");
    __asm__ volatile("ldr r3, =pxCurrentTCB");
    __asm__ volatile("ldr r2, [r3]");
    __asm__ volatile("stmdb r0!, {r4-r11}");  // save regs
    __asm__ volatile("str r0, [r2]");         // save TOS

    __asm__ volatile("stmdb sp!, {r3, r14}");
    __asm__ volatile("mov r0, #0");
    __asm__ volatile("msr basepri, r0");
    __asm__ volatile("dsb");
    __asm__ volatile("isb");
    __asm__ volatile("bl vTaskSwitchContext");

    __asm__ volatile("mov r0, #0");
    __asm__ volatile("msr basepri, r0");
    __asm__ volatile("ldmia sp!, {r3, r14}");
    __asm__ volatile("ldr r1, [r3]");
    __asm__ volatile("ldr r0, [r1]");
    __asm__ volatile("ldmia r0!, {r4-r11}");
    __asm__ volatile("msr psp, r0");
    __asm__ volatile("isb");
    __asm__ volatile("bx r14");
    __asm__ volatile("nop");
}


void vPortRaiseBASEPRI(void)
{
    uint32_t ulNewBASEPRI = prvCalcBASEPRI();

    __asm volatile(
        "msr basepri, %0\n"
        "dsb\n"
        "isb\n"
        :
        : "r" (ulNewBASEPRI)
        : "memory");
}

uint32_t ulPortRaiseBASEPRI(void)
{
    uint32_t ulReturn, ulNewBASEPRI = prvCalcBASEPRI();
    __asm volatile(
            "mrs %0, basepri\n"
            "msr basepri, %1\n"
            "dsb\n"
            "isb\n"
            : "=r" (ulReturn)
            : "r" (ulNewBASEPRI)
            : "memory");
    return ulReturn;
}
void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
    __asm__ volatile (
        "msr basepri, %0"
        :
        : "r" (ulBASEPRI)
        : "memory"
    );
}



void vPortEnterCritical(void)
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;

    if( uxCriticalNesting == 1)
    {
        configASSERT((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK   ) == 0);
    }
}


void vPortExitCritical(void)
{
    configASSERT(uxCriticalNesting);
    uxCriticalNesting--;
    if(uxCriticalNesting == 0)
    {
        portENABLE_INTERRUPTS();
    }
}


static void prvResetNextTaskUnblockTime(void)
{
    TCB_t *pxTCB;
    if(listLIST_IS_EMPTY(pxDelayedTaskList) != pdFALSE)
    {
        xNextTaskUnblockTime = configMAX_DELAY;
    }
    else
    {
        (pxTCB) = (TCB_t*)listGET_OWNER_OF_HEAD_ENTRY(pxDelayedTaskList);
        xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE(&(pxTCB->xStateListItem));
    }
}






void xTaskIncrementTick(void)
{
    TCB_t *pxTCB = NULL;
    uint32_t xItemValue;
    
    const uint32_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;

    if(xConstTickCount == (uint32_t) 0U)
    {
        taskSWITCH_DELAYED_LISTS();
    }

    if(xConstTickCount >= xNextTaskUnblockTime)
    {
        for(;;)
        {
            if(listLIST_IS_EMPTY(pxDelayedTaskList) != pdFALSE)
            {
                xNextTaskUnblockTime = configMAX_DELAY;
                break;
            }
            else
            {
                pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY(pxDelayedTaskList);
                xItemValue = listGET_LIST_ITEM_VALUE(&(pxTCB->xStateListItem));

                if(xConstTickCount < xItemValue)
                {
                    xNextTaskUnblockTime = xItemValue;
                    break;
                }

                (void)uxListRemove(&(pxTCB->xStateListItem));
                prvAddTaskToReadyList(pxTCB);
            }
        }
    }
#if 0
    for (uint32_t i = 0; i < configMAX_PRIORITIES; i++)
    {
        if(!listLIST_IS_EMPTY(&pxReadyTasksLists[i]))
        {
            /* code */
            pxTCB = (TCB_t*) listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
            if (pxTCB->xTicksToDelay > 0)
            {
                /* code */
                pxTCB->xTicksToDelay--;
                if(pxTCB->xTicksToDelay == 0)
                {
                    portRECORD_READY_PRIORITY(pxTCB->uxPriority,uxTopReadyPriority);
                }
            }
        }
    }
#endif
}

void xPortSysTickHandler(void)
{
    uint32_t old_mask;

    old_mask = taskENTER_CRITICAL_FROM_ISR();

    xTaskIncrementTick();

    taskEXIT_CRITICAL_FROM_ISR(old_mask);

    //临时存放
    portYIELD();
}


void HardFault_Handler(void)
{
    volatile uint32_t HFSR = SCB->HFSR;
    volatile uint32_t CFSR = SCB->CFSR;
    volatile uint32_t MMFAR = SCB->MMFAR;
    volatile uint32_t BFAR = SCB->BFAR;
    volatile uint32_t SHCSR = SCB->SHCSR;
    volatile uint32_t MSP, PSP, LR, PC;

    (void)HFSR;
    (void)CFSR;
    (void)MMFAR;
    (void)BFAR;
    (void)SHCSR;

    __asm volatile ("mrs %0, msp" : "=r" (MSP));   // 当前主栈指针
    __asm volatile ("mrs %0, psp" : "=r" (PSP));   // 任务栈指针
    __asm volatile ("mov %0, lr"  : "=r" (LR));    // 异常返回码
    __asm volatile ("mov %0, pc"  : "=r" (PC));    // 当前 PC

    // 在这里打断点 或 while(1) 让调试器停住
    for(;;);
}


void MemManage_Handler(void) __attribute__((alias("HardFault_Handler")));
void BusFault_Handler(void) __attribute__((alias("HardFault_Handler")));
void UsageFault_Handler(void) __attribute__((alias("HardFault_Handler")));

