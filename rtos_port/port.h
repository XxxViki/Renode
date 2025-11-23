#ifndef __PORT_H__
#define __PORT_H__
#include "task.h"
#include "projdefs.h"
#include "config.h"

#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY 0x0F
#endif

#define portINITIAL_XPSR    (0x01000000UL)    /* 初始xPSR寄存器值 */
#define portSTART_ADDRESS_MASK    (0xfffffffeUL)    /* 任务入口地址对齐掩码 */
#define portNVIC_SYSPRI2_REG    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_PENDSV_PRI    ( ( ( uint32_t ) 0x0F ) << 16UL )
#define portNVIC_SYSTICK_PRI    ( ( ( uint32_t ) 0x0F ) << 24UL )




#define taskYIELD()                portYIELD()
#define portNVIC_INT_CTRL_REG    ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )

#define portSY_FULL_READ_WRITE    ( 15 )

#define portYIELD()                              \
    {                                         \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
        __asm volatile("dsb");                   \
        __asm volatile("isb");                   \
    }


    
    
#define portNVIC_SYSTICK_CTRL_REG      (*((volatile uint32_t *) 0xe000e010))


#define portNVIC_SYSTICK_LOAD_REG      (*((volatile uint32_t *) 0xe000e014))


#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
#define portNVIC_SYSTICK_CLK_BIT    (1UL << 2UL)
#else
#define portNVIC_SYSTICK_CLK_BIT    (0)
#endif

#define portNVIC_SYSTICK_INT_BIT        (1UL << 1UL)
#define portNVIC_SYSTICK_ENABLE_BIT     (1UL << 1UL)


extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;
extern TCB_t Task_IDE_TCB;

#endif /* __PORT_H__ */