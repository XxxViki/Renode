// Cortex-M3 port glue: first-task startup and context switch handlers.
#include "port.h"

void vPortSVCHandler(void);
void vPortPendSVHandler(void);

void SVC_Handler(void) __attribute__((alias("vPortSVCHandler")));
void PendSV_Handler(void) __attribute__((alias("vPortPendSVHandler")));
void SysTick_Handler(void) __attribute__((alias("xPortSysTickHandler")));


static void prvStartFirstTask(void);
static void prvTaskExitError(void);

static uint32_t uxCriticalNesting = 0xaaaaaaaa;
uint32_t xTickCount = 0;

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
// configKERNEL_INTERRUPT_PRIORITY 是輸入變數，用於設置 BASEPRI 的值。
    uint32_t ulNewBASEPRI = configKERNEL_INTERRUPT_PRIORITY; 
    
    __asm volatile
    (
        // MSR 指令：將通用寄存器中的值寫入特殊寄存器 BASEPRI
        "   msr basepri, %0     \n"   // %0 會被替換為 ulNewBASEPRI 所在的寄存器
        "   dsb                 \n"   // 數據同步屏障，確保 MSR 完成
        "   isb                 \n"   // 指令同步屏障，刷新流水線
        : // 無輸出 (No output)
        : "r" (ulNewBASEPRI)           // 輸入約束："r" 表示將 ulNewBASEPRI 放入通用寄存器
        : "memory"                     // 破壞列表：通知編譯器內存可能被修改
    );
}


uint32_t ulPortRaiseBASEPRI(void)
{
    uint32_t ulReturn, ulNewBASEPRI = configKERNEL_INTERRUPT_PRIORITY;
    __asm volatile
        (
            "   mrs %0, basepri     \n"   // %0: 讀取舊的 BASEPRI 值到 ulReturn
            "   msr basepri, %1     \n"   // %1: 將 ulNewBASEPRI 寫入 BASEPRI
            "   dsb                 \n"   // 數據同步屏障
            "   isb                 \n"   // 指令同步屏障
            : "=r" (ulReturn)              // 輸出約束：ulReturn 存儲在通用寄存器中
            : "r" (ulNewBASEPRI)           // 輸入約束：ulNewBASEPRI 存儲在通用寄存器中
            : "memory"                     // 破壞列表（clobber list）
        );
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




void vTaskDelay(const uint32_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    pxTCB = pxCurrentTCB;

    pxTCB->xTicksToDelay = xTicksToDelay;

    taskRESET_READY_PRIORITY(pxTCB->uxPriority);

    taskYIELD();
}


void xTaskIncrementTick(void)
{
    TCB_t *pxTCB = NULL;
    
    const uint32_t xConstTickCount = xTickCount + 1;
    xTickCount = xConstTickCount;


    for (uint32_t i = 0; i < configMAX_PRIORITIES; i++)
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

void xPortSysTickHandler(void)
{
//    vPortRaiseBASEPRI();

    xTaskIncrementTick();

//    taskENTER_CRITICAL_FROM_ISR();

    //临时存放
    portYIELD();
}

