/**
 * @file		startup_stm23l432kc.c
 * @author		Andrew Loebs
 * @brief		Startup file for the application
 *
 * Defines the vector table, initializes static variables, zeros bss,
 * calls stdlib init, and the main() function.
 *
 * Heavily influenced by Miro Samek's startup file used in lesson 19
 * of his "Modern Embedded Systems Programming Course".
 * @see https://www.state-machine.com/quickstart/
 *
 */

#include <stdint.h>

// stack boundary
extern int __stack_start__;
extern int __stack_end__;

// function prototypes
void Default_Handler(void);
void Reset_Handler(void);
void assert_failed(char *file, uint32_t line);

// weak aliases for each exception handler to the Default_Handler.
void NMI_Handler(void) __attribute__((weak));
void HardFault_Handler(void) __attribute__((weak));
void MemManage_Handler(void) __attribute__((weak));
void BusFault_Handler(void) __attribute__((weak));
void UsageFault_Handler(void) __attribute__((weak));

void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void GPIOPortA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_PVM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void COMP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LPTIM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LPTIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void LPUART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void QUADSPI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SAI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SWPMI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TSC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

// vector table
__attribute__((section(".isr_vector"))) int const g_pfnVectors[] = {
    (int)&__stack_end__,
    (int)&Reset_Handler,
    (int)&NMI_Handler,
    (int)&HardFault_Handler,
    (int)&MemManage_Handler,
    (int)&BusFault_Handler,
    (int)&UsageFault_Handler,
    0,
    0,
    0,
    0,
    (int)&SVC_Handler,
    (int)&DebugMon_Handler,
    0,
    (int)&PendSV_Handler,
    (int)&SysTick_Handler,
    (int)&WWDG_IRQHandler,
    (int)&PVD_PVM_IRQHandler,
    (int)&TAMP_STAMP_IRQHandler,
    (int)&RTC_WKUP_IRQHandler,
    (int)&FLASH_IRQHandler,
    (int)&RCC_IRQHandler,
    (int)&EXTI0_IRQHandler,
    (int)&EXTI1_IRQHandler,
    (int)&EXTI2_IRQHandler,
    (int)&EXTI3_IRQHandler,
    (int)&EXTI4_IRQHandler,
    (int)&DMA1_Channel1_IRQHandler,
    (int)&DMA1_Channel2_IRQHandler,
    (int)&DMA1_Channel3_IRQHandler,
    (int)&DMA1_Channel4_IRQHandler,
    (int)&DMA1_Channel5_IRQHandler,
    (int)&DMA1_Channel6_IRQHandler,
    (int)&DMA1_Channel7_IRQHandler,
    (int)&ADC1_IRQHandler,
    (int)&CAN1_TX_IRQHandler,
    (int)&CAN1_RX0_IRQHandler,
    (int)&CAN1_RX1_IRQHandler,
    (int)&CAN1_SCE_IRQHandler,
    (int)&EXTI9_5_IRQHandler,
    (int)&TIM1_BRK_TIM15_IRQHandler,
    (int)&TIM1_UP_TIM16_IRQHandler,
    (int)&TIM1_TRG_COM_IRQHandler,
    (int)&TIM1_CC_IRQHandler,
    (int)&TIM2_IRQHandler,
    0,
    0,
    (int)&I2C1_EV_IRQHandler,
    (int)&I2C1_ER_IRQHandler,
    0,
    0,
    (int)&SPI1_IRQHandler,
    0,
    (int)&USART1_IRQHandler,
    (int)&USART2_IRQHandler,
    0,
    (int)&EXTI15_10_IRQHandler,
    (int)&RTC_Alarm_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (int)&SPI3_IRQHandler,
    0,
    0,
    (int)&TIM6_DAC_IRQHandler,
    (int)&TIM7_IRQHandler,
    (int)&DMA2_Channel1_IRQHandler,
    (int)&DMA2_Channel2_IRQHandler,
    (int)&DMA2_Channel3_IRQHandler,
    (int)&DMA2_Channel4_IRQHandler,
    (int)&DMA2_Channel5_IRQHandler,
    0,
    0,
    0,
    (int)&COMP_IRQHandler,
    (int)&LPTIM1_IRQHandler,
    (int)&LPTIM2_IRQHandler,
    (int)&USB_IRQHandler,
    (int)&DMA2_Channel6_IRQHandler,
    (int)&DMA2_Channel7_IRQHandler,
    (int)&LPUART1_IRQHandler,
    (int)&QUADSPI_IRQHandler,
    (int)&I2C3_EV_IRQHandler,
    (int)&I2C3_ER_IRQHandler,
    (int)&SAI1_IRQHandler,
    0,
    (int)&SWPMI1_IRQHandler,
    (int)&TSC_IRQHandler,
    0,
    0,
    (int)&RNG_IRQHandler,
    (int)&FPU_IRQHandler,
    (int)&CRS_IRQHandler,
};

// reset handler
void Reset_Handler(void)
{
    extern void SystemInit(void);
    extern int __libc_init_array(void);
    extern int main(void);

    extern unsigned __data_start;
    extern unsigned __data_end__;
    extern unsigned const __data_load;
    extern unsigned __bss_start__;
    extern unsigned __bss_end__;

    SystemInit();

    unsigned const *src;
    unsigned *dst;
    // copy the data segment initializers from flash to RAM
    src = &__data_load;
    for (dst = &__data_start; dst < &__data_end__; ++dst, ++src) {
        *dst = *src;
    }

    // zero fill the .bss segment in RAM
    for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
        *dst = 0;
    }

    __libc_init_array();
    (void)main();

    // we should never get here
    assert_failed("Reset_Handler", __LINE__);
}

void assert_failed(char *file, uint32_t line)
{
    // TODO: print file & line
    while (1) {
    }
}

// exception handlers
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void)
{
    __asm volatile("    ldr r0,=str_nmi\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_nmi: .asciz \"NMI\"\n\t"
                   "  .align 2\n\t");
}
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void)
{
    __asm volatile("    ldr r0,=str_mem\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_mem: .asciz \"MemManage\"\n\t"
                   "  .align 2\n\t");
}
__attribute__((naked)) void HardFault_Handler(void);
void HardFault_Handler(void)
{
    __asm volatile("    ldr r0,=str_hrd\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_hrd: .asciz \"HardFault\"\n\t"
                   "  .align 2\n\t");
}
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void)
{
    __asm volatile("    ldr r0,=str_bus\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_bus: .asciz \"BusFault\"\n\t"
                   "  .align 2\n\t");
}
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void)
{
    __asm volatile("    ldr r0,=str_usage\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_usage: .asciz \"UsageFault\"\n\t"
                   "  .align 2\n\t");
}
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void)
{
    __asm volatile("    ldr r0,=str_dflt\n\t"
                   "    mov r1,#1\n\t"
                   "    b assert_failed\n\t"
                   "str_dflt: .asciz \"Default\"\n\t"
                   "  .align 2\n\t");
}
