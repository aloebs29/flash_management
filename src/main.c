/**
 * @file		main.c
 * @author		Andrew Loebs
 * @brief		Main application	
 * 	
 */

#include <stdint.h>

#include "st/stm32_assert.h"
#include "st/ll/stm32l4xx_ll_bus.h"
#include "st/ll/stm32l4xx_ll_gpio.h"
#include "st/ll/stm32l4xx_ll_rcc.h"
#include "st/ll/stm32l4xx_ll_system.h"
#include "st/ll/stm32l4xx_ll_utils.h"

#include "modules/led.h"
#include "modules/sys_time.h"

#define STARTUP_LED_DURATION_MS         100
#define SYSTEM_CORE_CLOCK               80000000UL // Hz

// private function prototypes
static void clock_config(uint32_t system_core_clock_hz);

// application main function
int main(void)
{
    // setup clock
    clock_config(SYSTEM_CORE_CLOCK);

    // init modules
    led_init();
    sys_time_init(SYSTEM_CORE_CLOCK);

    // blink LED to let user know we're on
    led_set_output(true);
    sys_time_delay(STARTUP_LED_DURATION_MS);
    led_set_output(false);

    // main loop
    for (;;)
    {
        // tick functions
    }
}

// private function definitions
static void clock_config(uint32_t system_core_clock_hz)
{
    // for now this is only to catch future screw-ups
    if (80000000UL != system_core_clock_hz)
        assert_failed(__FILE__, __LINE__);

    // enable prefetch & set latency -- icache and dcache are enabled by default
    LL_FLASH_EnablePrefetch(); // according to the ref manual, this negates perf impact due to flash latency
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    // enable MSI
    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1); // TODO: add timeouts to these while loops

    // pll config & enable
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while (LL_RCC_PLL_IsReady() != 1);

    // sys clk activation
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    // setup APB1 & APB2
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    // update CMSIS variable
    LL_SetSystemCoreClock(system_core_clock_hz);
}