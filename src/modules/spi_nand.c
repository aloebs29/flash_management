/**
 * @file		spi_nand.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the spi nand module
 *
 */

#include "spi_nand.h"

#include <stdbool.h>
#include <stdint.h>

#include "../st/ll/stm32l4xx_ll_bus.h"
#include "../st/ll/stm32l4xx_ll_gpio.h"

#include "spi.h"
#include "sys_time.h"

// defines
#define CSEL_PORT GPIOB
#define CSEL_PIN  LL_GPIO_PIN_0

#define RESET_DELAY 2    // ms
#define OP_TIMEOUT  3000 // ms

#define CMD_RESET       0xFF
#define CMD_READ_ID     0x9F
#define CMD_SET_FEATURE 0x1F

#define READ_ID_TRANS_LEN    4
#define READ_ID_MFR_INDEX    2
#define READ_ID_DEVICE_INDEX 3
#define MFR_ID_MICRON        0x2C
#define DEVICE_ID_1G_3V3     0x14

#define FEATURE_TRANS_LEN  3
#define FEATURE_REG_INDEX  1
#define FEATURE_DATA_INDEX 2

#define REG_STATUS 0xC0
#define OIP_BIT    0

#define REG_BLOCK_LOCK    0xA0
#define UNLOCK_ALL_BLOCKS 0x00

#define REG_ECC_EN 0xB0
#define ECC_ENABLE 1 << 4

// private function prototypes
static void csel_setup(void);
static void csel_deselect(void);
static void csel_select(void);

static int reset(void);
static int read_id(void);
static int unlock_all_blocks(void);
static int enable_ecc(void);

static int set_feature(uint8_t reg, uint8_t data);
static int get_feature(uint8_t reg, uint8_t *data_out);

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

    // read id
    ret = read_id();
    if (SPI_NAND_RET_OK != ret) return ret; // exit upon error

    // unlock all blocks
    ret = unlock_all_blocks();
    if (SPI_NAND_RET_OK != ret) return ret; // exit upon error

    // enable ecc
    ret = enable_ecc();
    if (SPI_NAND_RET_OK != ret) return ret; // exit upon error

    return ret;
}

// private function definitions
static void csel_setup(void)
{
    // enable peripheral clock
    if (!LL_AHB2_GRP1_IsEnabledClock(LL_AHB2_GRP1_PERIPH_GPIOB))
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

    // setup pin as output
    LL_GPIO_SetPinMode(CSEL_PORT, CSEL_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(CSEL_PORT, CSEL_PIN, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(CSEL_PORT, CSEL_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(CSEL_PORT, CSEL_PIN, LL_GPIO_PULL_NO);
}

static void csel_deselect(void)
{
    LL_GPIO_SetOutputPin(CSEL_PORT, CSEL_PIN);
}

static void csel_select(void)
{
    LL_GPIO_ResetOutputPin(CSEL_PORT, CSEL_PIN);
}

static int reset(void)
{
    // setup data
    uint8_t tx_data = CMD_RESET; // this is just a one-byte command
    // perform transaction
    csel_select();
    int ret = spi_write(&tx_data, 1, OP_TIMEOUT);
    csel_deselect();
    // exit if bad status
    if (SPI_RET_OK != ret) return SPI_NAND_RET_BAD_SPI;

    // Poll for OIP bit clear
    uint32_t start_time = sys_time_get_ms();
    for (;;) {
        uint8_t status;
        ret = get_feature(REG_STATUS, &status);
        // break on bad return -- keep this ret value to bubble up the error code
        if (SPI_NAND_RET_OK != ret) {
            break;
        }
        // check for OIP clear (which means reset is done)
        if (0 == (status & OIP_BIT)) {
            ret = SPI_NAND_RET_OK;
            break;
        }
        // check for timeout
        if (sys_time_is_elapsed(start_time, OP_TIMEOUT)) {
            ret = SPI_NAND_RET_TIMEOUT;
            break;
        }
    }

    return ret;
}

static int read_id(void)
{
    // setup data
    uint8_t tx_data[READ_ID_TRANS_LEN] = {0};
    uint8_t rx_data[READ_ID_TRANS_LEN] = {0};
    tx_data[0] = CMD_READ_ID;
    // perform transaction
    csel_select();
    int ret = spi_write_read(tx_data, rx_data, READ_ID_TRANS_LEN, OP_TIMEOUT);
    csel_deselect();

    // check spi return
    if (SPI_RET_OK == ret) {
        // check mfr & device id
        if ((MFR_ID_MICRON == rx_data[READ_ID_MFR_INDEX]) &&
            (DEVICE_ID_1G_3V3 == rx_data[READ_ID_DEVICE_INDEX])) {
            // success
            return SPI_NAND_RET_OK;
        }
        else {
            // bad mfr or device id
            return SPI_NAND_RET_DEVICE_ID;
        }
    }
    else {
        return SPI_NAND_RET_BAD_SPI;
    }
}

static int unlock_all_blocks(void)
{
    return set_feature(REG_BLOCK_LOCK, UNLOCK_ALL_BLOCKS);
}

static int enable_ecc(void)
{
    return set_feature(REG_ECC_EN, ECC_ENABLE); // we want to zero the other bits here
}

static int set_feature(uint8_t reg, uint8_t data)
{
    // setup data
    uint8_t tx_data[FEATURE_TRANS_LEN] = {0};
    tx_data[0] = CMD_SET_FEATURE;
    tx_data[FEATURE_REG_INDEX] = reg;
    tx_data[FEATURE_DATA_INDEX] = data;
    // perform transaction
    csel_select();
    int ret = spi_write(tx_data, FEATURE_TRANS_LEN, OP_TIMEOUT);
    csel_deselect();

    // map spi return to spi nand return
    return (SPI_RET_OK == ret) ? SPI_NAND_RET_OK : SPI_NAND_RET_BAD_SPI;
}

static int get_feature(uint8_t reg, uint8_t *data_out)
{
    // setup data
    uint8_t tx_data[FEATURE_TRANS_LEN] = {0};
    uint8_t rx_data[FEATURE_TRANS_LEN] = {0};
    tx_data[0] = CMD_SET_FEATURE;
    tx_data[FEATURE_REG_INDEX] = reg;
    // perform transaction
    csel_select();
    int ret = spi_write_read(tx_data, rx_data, FEATURE_TRANS_LEN, OP_TIMEOUT);
    csel_deselect();

    // if good return, write data out
    if (SPI_RET_OK == ret) {
        *data_out = rx_data[FEATURE_DATA_INDEX];
        return SPI_NAND_RET_OK;
    }
    else {
        return SPI_NAND_RET_BAD_SPI;
    }
}