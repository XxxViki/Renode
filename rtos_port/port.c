// Cortex-M3 port glue: first-task startup and context switch handlers.
#include "port.h"

void vPortSVCHandler(void);
void vPortPendSVHandler(void);

void SVC_Handler(void) __attribute__((alias("vPortSVCHandler")));
void PendSV_Handler(void) __attribute__((alias("vPortPendSVHandler")));

static void prvStartFirstTask(void);
static void prvTaskExitError(void);

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

void xPortStartScheduler(void)
{
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
    prvStartFirstTask();
}

void vTaskSwitchContext(void)
{
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
    __asm__ volatile("msr msp, r0");
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
