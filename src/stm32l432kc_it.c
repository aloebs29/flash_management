/**
 * @file		stm32l432_it.c
 * @author		Andrew Loebs
 * @brief		Interrupt handler definitions
 *
 * Based on ST's LL template project. All handlers defined here will override
 * their weak aliases in the startup file.
 *
 */

#include "modules/led.h"
#include "modules/sys_time.h"
#include "modules/uart.h"

void NMI_Handler(void)
{
    // TODO: write production-worthy exception handlers
    led_set_output(true);
    while (1)
        ;
}

void HardFault_Handler(void)
{
    led_set_output(true);
    while (1)
        ;
}

void MemManage_Handler(void)
{
    led_set_output(true);
    while (1)
        ;
}

void BusFault_Handler(void)
{
    led_set_output(true);
    while (1)
        ;
}

void UsageFault_Handler(void)
{
    led_set_output(true);
    while (1)
        ;
}

void SysTick_Handler(void) { _sys_time_increment(); }

void USART2_IRQHandler(void) { _uart_isr(); }
