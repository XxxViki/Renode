#include <stdio.h>
#include <stdint.h>

uint32_t tmp_var = 0;

int main(void)
{
    while(1)
    {
        tmp_var++;
        for (size_t i = 0; i < 10000; i++)
        {
            /* code */
            asm volatile("nop");
        }
    }
}