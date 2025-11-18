    .syntax unified
    .cpu cortex-m3
    .thumb

    .extern main
    .extern SystemInit

    .global g_pfnVectors
    .global Reset_Handler

/* ---------------------------
 *  向量表
 * -------------------------- */
    .section .isr_vector, "a", %progbits
g_pfnVectors:
    .word   _estack
    .word   Reset_Handler
    .word   NMI_Handler
    .word   HardFault_Handler
    .word   MemManage_Handler
    .word   BusFault_Handler
    .word   UsageFault_Handler
    .word   0
    .word   0
    .word   0
    .word   0
    .word   SVC_Handler
    .word   DebugMon_Handler
    .word   0
    .word   PendSV_Handler
    .word   SysTick_Handler

/* 以下外设中断可以按需扩展 */
    .word   WWDG_Handler
    .word   PVD_Handler
    .word   TAMPER_Handler
    .word   RTC_Handler
    .word   FLASH_Handler
    .word   RCC_Handler
    .word   EXTI0_Handler
    .word   EXTI1_Handler
    .word   EXTI2_Handler
    .word   EXTI3_Handler
    .word   EXTI4_Handler


/* ---------------------------
 * Reset_Handler
 * -------------------------- */
    .section .text.Reset_Handler, "ax", %progbits
    .thumb_func
Reset_Handler:
    cpsid   i          /* 禁用中断 */

    bl      SystemInit /* 系统初始化 */

    /* 初始化 .data 段 */
    ldr     r0, =_sidata
    ldr     r1, =_sdata
    ldr     r2, =_edata
1:
    cmp     r1, r2
    bcc     2f        /* if r1 < r2 jump to copy */
    b       3f
2:
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    b       1b

3:
    /* 清零 .bss */
    ldr     r1, =_sbss
    ldr     r2, =_ebss
4:
    cmp     r1, r2
    bcc     5f
    b       6f
5:
    movs    r3, #0
    str     r3, [r1], #4
    b       4b

6:
    cpsie   i         /* 开启中断 */
    bl      main      /* 进入 C 程序 */

7:
    b       7b         /* 如果 main 返回，就死循环 */


/* ---------------------------
 * 默认弱定义的中断处理函数
 * -------------------------- */
    .weak NMI_Handler
NMI_Handler: b .

    .weak HardFault_Handler
HardFault_Handler: b .

    .weak MemManage_Handler
MemManage_Handler: b .

    .weak BusFault_Handler
BusFault_Handler: b .

    .weak UsageFault_Handler
UsageFault_Handler: b .

    .weak SVC_Handler
SVC_Handler: b .

    .weak DebugMon_Handler
DebugMon_Handler: b .

    .weak PendSV_Handler
PendSV_Handler: b .

    .weak SysTick_Handler
SysTick_Handler: b .

    .weak WWDG_Handler
WWDG_Handler: b .
    .weak PVD_Handler
PVD_Handler: b .
    .weak TAMPER_Handler
TAMPER_Handler: b .
    .weak RTC_Handler
RTC_Handler: b .
    .weak FLASH_Handler
FLASH_Handler: b .
    .weak RCC_Handler
RCC_Handler: b .
    .weak EXTI0_Handler
EXTI0_Handler: b .
    .weak EXTI1_Handler
EXTI1_Handler: b .
    .weak EXTI2_Handler
EXTI2_Handler: b .
    .weak EXTI3_Handler
EXTI3_Handler: b .
    .weak EXTI4_Handler
EXTI4_Handler: b .
