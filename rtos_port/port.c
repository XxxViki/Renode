#include "port.h"

extern void vPortSVCHandler(void);
extern void vPortPendSVHandler(void);
// extern void xPortSysTickHandler(void);

void SVC_Handler(void)      __attribute__((alias("vPortSVCHandler")));
void PendSV_Handler(void)   __attribute__((alias("vPortPendSVHandler")));
// void SysTick_Handler(void)  __attribute__((alias("xPortSysTickHandler")));


void prvStartFirstTask(void);
void vPortSVCHandler(void);
void vPortPendSVHandler(void);



static void prvTaskExitError(void)
{
    /* 任务不应退出 */
    for( ;; );
}

uint32_t *rtosPortInitialiseStack( uint32_t *pxTopOfStack, 
                                    TaskFunction_t pxCode, 
                                    void *pvParameters )
{
    /* 异常发生时，自动加载到CPU寄存器的内容*/
    pxTopOfStack--;                                 // 占位，模拟异常发生时的栈帧
    *pxTopOfStack = portINITIAL_XPSR;               
    pxTopOfStack--;                                 // 占位，模拟异常发生时的栈帧
    *pxTopOfStack = ((uint32_t)pxCode) & portSTART_ADDRESS_MASK; // PC
    pxTopOfStack--;                                 // 占位，模拟异常发生时的栈帧
    *pxTopOfStack = (uint32_t)prvTaskExitError;     // LR
    pxTopOfStack -= 5;                              /* R12,R3,R2 and R1 默认初始化值为0*/
    *pxTopOfStack = (uint32_t)pvParameters;        // R0 参数

    pxTopOfStack -= 8;                              /* R11,R10,R9,R8,R7,R6,R5 and R4 默认初始化值为0*/  

    return pxTopOfStack;
}

void xPortStartScheduler(void)
{
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;

    prvStartFirstTask();

}


void vTaskSwitchContext( void )
{
    if(pxCurrentTCB == &Task1_TCB)
    {
        pxCurrentTCB = &Task2_TCB;
    }
    else if(pxCurrentTCB == &Task2_TCB)
    {
        pxCurrentTCB = &Task1_TCB;
    }
    else
    {
        pxCurrentTCB = &Task1_TCB;
    }
}

__attribute__((naked)) void prvStartFirstTask(void)
{
    __asm__ __volatile__(
        " PRESERVE8                 \n"
        "ldr r0, =0xE000ED08    \n" // 获取当前TCB指针
        "ldr r0, [r0]            \n"
        "ldr r0, [r0]            \n"
        "msr msp, r0             \n" // 设置主堆栈指针
        "cpsie i                 \n" // 使能中断
        "cpsie f                 \n"
        "dsb                     \n"
        "isb                     \n"

        "svc 0                   \n" // 触发SVC中断，跳转到任务入口
        "nop                     \n"
        "nop                     \n"
    );
}

extern TCB_t *pxCurrentTCB;
__attribute__((naked)) void vPortSVCHandler(void)
{
    __asm__ __volatile__(
        " PRESERVE8                 \n"

        "ldr r3, =pxCurrentTCB    \n"
        "ldr r1, [r3]            \n"
        "ldr r0, [r1]            \n" // 获取任务栈顶指针
        "ldmia r0!, {r4-r11}       \n" // 恢复任务寄存器
        "msr msp, r0                \n" // 设置主堆栈指针
        "isb                     \n"
        "mov r0, #0             \n"
        "msr basepri, r0        \n" // 使能中断
        "orr r14, #0xd          \n" // 从异常返回，使用主堆栈
        "bx r14                 \n"
    );
}





__attribute__((naked)) void vPortPendSVHandler(void)
{
    __asm__ __volatile__(
        " PRESERVE8                 \n"

        "mrs r0, psp             \n" // 获取当前任务栈顶指针
        "isb                     \n"
        "ldr r3, =pxCurrentTCB    \n"
        "ldr r2, [r3]            \n"
        "stmdb r0!, {r4-r11}      \n" // 保存任务寄存器
        "str r0, [r2]            \n" // 保存任务栈顶指针
        
        "stmdb sp!,{r3,r14}"
        "mov r0, #0             \n"
        "msr basepri, r0        \n" // 使能中断
        "dsb                 \n"
        "isb                 \n"
        
        "bl vTaskSwitchContext \n" 

        "mov r0, #0             \n"

        "msr basepri, r0        \n" // 使能中断
        "ldmia sp!,{r3,r14}"
        "ldr r1, [r3]            \n"
        "ldr r0, [r1]            \n" // 获取任务栈顶指针
        "ldmia r0!, {r4-r11}     \n" // 恢复任务寄存器
        "msr psp, r0            \n" // 设置任务栈顶指针
        "isb                     \n"
        "bx r14                 \n"
        "nop"
    );
}


