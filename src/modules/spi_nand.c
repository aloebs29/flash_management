/**
 * @file		spi_nand.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the spi nand module
 *
 */

#include "spi_nand.h"

#include <stdbool.h>
#include <stdint.h>

#include "../st/ll/stm32l4xx_ll_gpio.h"
#include "spi.h"
#include "sys_time.h"

// defines
#define CSEL_PORT GPIOA
#define CSEL_PIN  LL_GPIO_PIN_8

#define RESET_DELAY 2    // ms
#define OP_TIMEOUT  3000 // ms

#define CMD_RESET 0xFF

// private function prototypes
static void csel_setup(void);
static void csel_deselect(void);
static void csel_select(void);

static int reset(void);

// public function definitions
int spi_nand_init(void)
{
    // setup csel and set high
    csel_deselect();
    csel_setup();

    // reset
    sys_time_delay(RESET_DELAY);
    int ret = reset();
    if (SPI_NAND_RET_OK != ret) return ret; // exit upon error
    sys_time_delay(RESET_DELAY);

    return ret;
}

// private function definitions
static void csel_setup(void)
{
    LL_GPIO_SetPinMode(CSEL_PORT, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(CSEL_PORT, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(CSEL_PORT, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(CSEL_PORT, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
}

static void csel_deselect(void) { LL_GPIO_SetOutputPin(CSEL_PORT, CSEL_PIN); }

static void csel_select(void) { LL_GPIO_ResetOutputPin(CSEL_PORT, CSEL_PIN); }

static int reset(void)
{
    // setup data
    uint8_t tx_data = CMD_RESET; // this is just a one-byte command
    // perform transaction
    csel_select();
    int ret = spi_write(&tx_data, 1, OP_TIMEOUT);
    csel_deselect();

    // map spi return to spi nand return
    return (SPI_RET_OK == ret) ? SPI_NAND_RET_OK : SPI_NAND_RET_BAD_SPI;
}
