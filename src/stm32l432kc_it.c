/**
 * @file		stm32l432_it.c
 * @author		Andrew Loebs
 * @brief		Interrupt handler definitions
 * 
 * Based on ST's LL template project. All handlers defined here will override
 * their weak aliases in the startup file.
 * 	
 */

#include "modules/led.h" // to set LED for exception handlers
#include "modules/sys_time.h" // for incrementing sys time every sys tick interrupt
#include "modules/uart.h"

// fault exception handlers
void NMI_Handler(void)
{
    // TODO: write production-worthy exception handlers
    led_set_output(true); // to make it obvious that an exception occurred
    while (1);
}

void HardFault_Handler(void)
{
    led_set_output(true); // to make it obvious that an exception occurred
    while (1);
}

void MemManage_Handler(void)
{
    led_set_output(true); // to make it obvious that an exception occurred
    while (1);
}

void BusFault_Handler(void)
{
    led_set_output(true); // to make it obvious that an exception occurred
    while (1);
}

void UsageFault_Handler(void)
{
    led_set_output(true); // to make it obvious that an exception occurred
    while (1);
}

// non-fault exception handlers
void SysTick_Handler(void)
{
    _sys_time_increment();
}

// peripheral interrupt handlers
void USART2_IRQHandler(void)
{
    _uart_isr();
}