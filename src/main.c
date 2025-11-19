#include <stdint.h>

#define PERIPH_BASE        0x40000000UL
#define APB1PERIPH_BASE    (PERIPH_BASE + 0x00000)
#define USART2_BASE        (APB1PERIPH_BASE + 0x4400)

#define USART2_SR   (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_DR   (*(volatile uint32_t*)(USART2_BASE + 0x04))
#define USART2_BRR  (*(volatile uint32_t*)(USART2_BASE + 0x08))
#define USART2_CR1  (*(volatile uint32_t*)(USART2_BASE + 0x0C))


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

int main(void)
{
    uart2_init();

    while(1)
    {
        uart2_send_string("Hello from USART2!\r\n");
        for(volatile int i = 0; i < 300000; i++);  // 简单延时
    }
}