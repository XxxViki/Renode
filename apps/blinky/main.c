#include <stdint.h>
#include "task.h"
#include "list.h"
#include "port.h"

#define PERIPH_BASE        0x40000000UL
#define APB1PERIPH_BASE    (PERIPH_BASE + 0x00000)
#define USART2_BASE        (APB1PERIPH_BASE + 0x4400)

#define USART2_SR   (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_DR   (*(volatile uint32_t*)(USART2_BASE + 0x04))
#define USART2_BRR  (*(volatile uint32_t*)(USART2_BASE + 0x08))
#define USART2_CR1  (*(volatile uint32_t*)(USART2_BASE + 0x0C))

TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE  128
uint32_t Task1_Stack[TASK1_STACK_SIZE];

TCB_t Task1_TCB;
TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE  128
uint32_t Task2_Stack[TASK2_STACK_SIZE];
TCB_t Task2_TCB;


void delay(uint32_t count);
void* Task1_Entry(void *pvParameters);
void* Task2_Entry(void *pvParameters);

void vTaskStartScheduler(void);

void uart2_init(void)
{
    USART2_CR1 |= (1 << 13);  // UE: USART enable
    USART2_CR1 |= (1 << 3);   // TE: Transmitter enable

    USART2_BRR = 0x1D4C;      // 任意值即可（Renode 不依赖它）
}

void uart2_send_char(char c)
{
    while(!(USART2_SR & (1 << 7)));  // TXE
    USART2_DR = c;
}

void uart2_send_string(const char *s)
{
    while(*s)
    {
        uart2_send_char(*s++);
    }
}


void delay(uint32_t count)
{
    for(volatile uint32_t i = 0; i < count; i++);
}

void* Task1_Entry(void *pvParameters)
{
    while(1)
    {
#if 0
        uart2_send_string("Task 1 is running.\r\n");
        delay(1000000);
        portYIELD();
#endif
        vTaskDelay(20);
        uart2_send_string("Task 1 is running.\r\n");
        vTaskDelay(20);
    }
}

void* Task2_Entry(void *pvParameters)
{
    while(1)
    {
#if 0        
        uart2_send_string("Task 2 is running.\r\n");
        delay(1000000);
        portYIELD();
#endif
        vTaskDelay(100);
        uart2_send_string("Task 2 is running.\r\n");
        vTaskDelay(20);
    }
}

int main(void)
{
    uart2_init();

    prvInitializeTaskLists();
    Task1_Handle = xTaskCreateStatic(
        Task1_Entry,
        "Task1",
        TASK1_STACK_SIZE,
        NULL,
        1,
        Task1_Stack,
        &Task1_TCB
    );
    
    // rtosListInsertEnd(&pxReadyTasksLists[0], &Task1_TCB.xStateListItem);
    Task2_Handle = xTaskCreateStatic(
        Task2_Entry,
        "Task2",
        TASK2_STACK_SIZE,
        NULL,
        2,
        Task2_Stack,
        &Task2_TCB
    );
    // rtosListInsertEnd(&pxReadyTasksLists[1], &Task2_TCB.xStateListItem);
    

    pxCurrentTCB = &Task2_TCB;
    vTaskStartScheduler();
    

    while(1)
    {
        uart2_send_string("Hello from USART2!\r\n");
        for(volatile int i = 0; i < 300000; i++);  // 简单延时
    }
}





void vApplicationGetIdleTaskMemory(TCB_t **ppxIdleTaskTCBBuffer,
                                    uint32_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &Task_IDE_TCB;
    *ppxIdleTaskStackBuffer = Task_IDE_Stack;
    *pulIdleTaskStackSize = configMINIAL_STACK_SIZE;
}


void vTaskStartScheduler(void)
{
    TCB_t *pxIdleTaskTCBBuffer = NULL;
    uint32_t *pxIdleTaskStackBuffer = NULL;
    uint32_t ulIdleTaskStackSize;

    uxCriticalNesting = 0;

    vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer,
                                    &pxIdleTaskStackBuffer,
                                    &ulIdleTaskStackSize );

    Task_IDE_Handle = xTaskCreateStatic(
        Task_IDE_Entry,
        "Task_IDE",
        ulIdleTaskStackSize,
        NULL,
        0,
        pxIdleTaskStackBuffer,
        pxIdleTaskTCBBuffer
    );

    xPortStartScheduler();
}
