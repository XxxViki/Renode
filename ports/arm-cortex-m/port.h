#ifndef __PORT_H__
#define __PORT_H__
#include "stm32f4xx.h"  /* Provides IRQn_Type and __NVIC_PRIO_BITS */
#include "task.h"
#include "projdefs.h"
#include "config.h"
#include "list.h"
#include "core_cm4.h"   // 或 core_cm3.h，根据你的 MCU 核心




extern TCB_t Task1_TCB;
extern TCB_t Task2_TCB;
extern TCB_t Task_IDE_TCB;

#endif /* __PORT_H__ */
