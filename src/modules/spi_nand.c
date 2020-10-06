/**
 * @file		spi_nand.c
 * @author		Andrew Loebs
 * @brief		Implementation file of the spi nand module
 *
 */

#include "spi_nand.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../st/ll/stm32l4xx_ll_bus.h"
#include "../st/ll/stm32l4xx_ll_gpio.h"

#include "spi.h"
#include "sys_time.h"

// defines
#define CSEL_PORT GPIOB
#define CSEL_PIN  LL_GPIO_PIN_0

#define RESET_DELAY 2    // ms
#define OP_TIMEOUT  3000 // ms

#define CMD_RESET           0xFF
#define CMD_READ_ID         0x9F
#define CMD_SET_FEATURE     0x1F
#define CMD_GET_FEATURE     0x0F
#define CMD_PAGE_READ       0x13
#define CMD_READ_FROM_CACHE 0x03
#define CMD_WRITE_ENABLE    0x06
#define CMD_PROGRAM_LOAD    0x02
#define CMD_PROGRAM_EXECUTE 0x10

#define READ_ID_TRANS_LEN    4
#define READ_ID_MFR_INDEX    2
#define READ_ID_DEVICE_INDEX 3
#define MFR_ID_MICRON        0x2C
#define DEVICE_ID_1G_3V3     0x14

#define FEATURE_TRANS_LEN  3
#define FEATURE_REG_INDEX  1
#define FEATURE_DATA_INDEX 2

#define PAGE_READ_TRANS_LEN       4
#define READ_FROM_CACHE_TRANS_LEN 4
#define PROGRAM_LOAD_TRANS_LEN    3
#define PROGRAM_EXECUTE_TRANS_LEN 4

#define FEATURE_REG_STATUS        0xC0
#define FEATURE_REG_BLOCK_LOCK    0xA0
#define FEATURE_REG_CONFIGURATION 0xB0
#define FEATURE_REG_DIE_SELECT    0xC0

#define ECC_STATUS_NO_ERR         0b000
#define ECC_STATUS_1_3_NO_REFRESH 0b001
#define ECC_STATUS_4_6_REFRESH    0b011
#define ECC_STATUS_7_8_REFRESH    0b101
#define ECC_STATUS_NOT_CORRECTED  0b010

#define ROW_ADDRESS_BLOCK_SHIFT 6

// private types
typedef union {
    uint8_t whole;
    struct {
        uint8_t : 1;
        uint8_t WP_HOLD_DISABLE : 1;
        uint8_t TB : 1;
        uint8_t BP0 : 1;
        uint8_t BP1 : 1;
        uint8_t BP2 : 1;
        uint8_t BP3 : 1;
        uint8_t BRWD : 1;
    };
} feature_reg_block_lock_t;

typedef union {
    uint8_t whole;
    struct {
        uint8_t : 1;
        uint8_t CFG0 : 1;
        uint8_t : 2;
        uint8_t ECC_EN : 1;
        uint8_t LOT_EN : 1;
        uint8_t CFG1 : 1;
        uint8_t CFG2 : 1;
    };
} feature_reg_configuration_t;

typedef union {
    uint8_t whole;
    struct {
        uint8_t OIP : 1;
        uint8_t WEL : 1;
        uint8_t E_FAIL : 1;
        uint8_t P_FAIL : 1;
        uint8_t ECCS0_3 : 3;
        uint8_t CRBSY : 1;
    };
} feature_reg_status_t;

typedef union {
    uint8_t whole;
    struct {
        uint8_t : 6;
        uint8_t DS0 : 1;
        uint8_t : 1;
    };
} feature_reg_die_select_t;

// private function prototypes
static void csel_setup(void);
static void csel_deselect(void);
static void csel_select(void);

static int reset(void);
static int read_id(void);
static int unlock_all_blocks(void);
static int enable_ecc(void);

static int set_feature(uint8_t reg, uint8_t data, uint32_t timeout);
static int get_feature(uint8_t reg, uint8_t *data_out, uint32_t timeout);

static bool validate_row_address(block_address_t block_address, page_address_t page_address);
static bool validate_column_address(column_address_t address);
static int poll_for_oip_clear(uint32_t timeout);
static int get_ecc_status(uint32_t timeout);

static int write_enable(uint32_t timeout);

// public function definitions
int spi_nand_init(void)
{
    // initialize chip select
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

int spi_nand_page_read(block_address_t block, page_address_t page, column_address_t column,
                       uint8_t *data_out, size_t data_out_len)
{
    // input validation
    if (!validate_row_address(block, page) || !validate_column_address(column)) {
        return SPI_NAND_RET_BAD_ADDRESS;
    }
    uint16_t read_len = (SPI_NAND_PAGE_SIZE + SPI_NAND_OOB_SIZE) - column;
    if (data_out_len < read_len) read_len = data_out_len; // truncate to the caller's buffer size

    // setup timeout tracking
    uint32_t start = sys_time_get_ms();

    // setup data for page read command (need to go from LSB -> MSB first on address)
    uint32_t row_address = ((uint32_t)block << ROW_ADDRESS_BLOCK_SHIFT) + page;
    uint8_t page_read_tx_data[PAGE_READ_TRANS_LEN];
    page_read_tx_data[0] = CMD_PAGE_READ;
    page_read_tx_data[1] = row_address >> 16;
    page_read_tx_data[2] = row_address >> 8;
    page_read_tx_data[3] = row_address;
    // perform transaction
    csel_select();
    uint32_t timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    int ret = spi_write(page_read_tx_data, PAGE_READ_TRANS_LEN, timeout);
    csel_deselect();
    // exit if bad status
    if (SPI_RET_OK != ret) return SPI_NAND_RET_BAD_SPI;

    // wait until that operation finishes
    timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    ret = poll_for_oip_clear(timeout);
    // exit if bad status
    if (SPI_RET_OK != ret) return ret;

    // check ecc
    timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    int ecc_status_ret = get_ecc_status(timeout);

    // setup data for page read command (need to go from LSB -> MSB first on address)
    uint8_t read_from_cache_tx_data[READ_FROM_CACHE_TRANS_LEN];
    read_from_cache_tx_data[0] = CMD_READ_FROM_CACHE;
    read_from_cache_tx_data[1] = column >> 8;
    read_from_cache_tx_data[2] = column;
    read_from_cache_tx_data[3] = 0;
    // perform transaction
    csel_select();
    timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    ret = spi_write(read_from_cache_tx_data, READ_FROM_CACHE_TRANS_LEN, timeout);
    if (SPI_RET_OK == ret) {
        timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
        ret = spi_read(data_out, read_len, timeout);
    }
    csel_deselect();

    /* Determine return status. We've accumulated the ecc status, and spi status from both the
     * write and read op. Preference will be given to return bad SPI statuses so that the caller is
     * aware that the return data is bad. */
    if (SPI_RET_OK != ret) {
        return ret;
    }
    else {
        return ecc_status_ret;
    }
}

int spi_nand_page_program(block_address_t block, page_address_t page, column_address_t column,
                          uint8_t *data_in, size_t data_in_len)
{
    // input validation
    if (!validate_row_address(block, page) || !validate_column_address(column)) {
        return SPI_NAND_RET_BAD_ADDRESS;
    }
    uint16_t max_write_len = (SPI_NAND_PAGE_SIZE + SPI_NAND_OOB_SIZE) - column;
    if (data_in_len > max_write_len) return SPI_NAND_RET_BUFFER_LEN;

    // setup timeout tracking
    uint32_t start = sys_time_get_ms();

    // write enable
    int ret = write_enable(OP_TIMEOUT); // ignore the time elapsed since start since its negligible
    // exit if bad status
    if (SPI_NAND_RET_OK != ret) return ret;

    // setup data for program load (need to go from LSB -> MSB first on address)
    uint8_t program_load_tx_data[PROGRAM_LOAD_TRANS_LEN];
    program_load_tx_data[0] = CMD_PROGRAM_LOAD;
    program_load_tx_data[1] = column >> 8;
    program_load_tx_data[2] = column;
    // perform transaction
    csel_select();
    uint32_t timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    ret = spi_write(program_load_tx_data, PROGRAM_LOAD_TRANS_LEN, timeout);
    if (SPI_RET_OK == ret) {
        timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
        ret = spi_write(data_in, data_in_len, timeout);
    }
    csel_deselect();
    // exit if bad status
    if (SPI_RET_OK != ret) return SPI_NAND_RET_BAD_SPI;

    // setup data for program execute (need to go from LSB -> MSB first on address)
    uint32_t row_address = ((uint32_t)block << ROW_ADDRESS_BLOCK_SHIFT) + page;
    uint8_t program_execute_tx_data[PROGRAM_EXECUTE_TRANS_LEN];
    program_execute_tx_data[0] = CMD_PROGRAM_EXECUTE;
    program_execute_tx_data[1] = row_address >> 16;
    program_execute_tx_data[2] = row_address >> 8;
    program_execute_tx_data[3] = row_address;
    // perform transaction
    csel_select();
    timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    ret = spi_write(program_execute_tx_data, PAGE_READ_TRANS_LEN, timeout);
    csel_deselect();
    // exit if bad status
    if (SPI_RET_OK != ret) return SPI_NAND_RET_BAD_SPI;

    // wait until that operation finishes
    timeout = OP_TIMEOUT - sys_time_get_elapsed(start);
    return poll_for_oip_clear(timeout);
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

    // wait until op is done or we timeout
    return poll_for_oip_clear(OP_TIMEOUT);
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
    feature_reg_block_lock_t unlock_all = {.whole = 0};
    return set_feature(FEATURE_REG_BLOCK_LOCK, unlock_all.whole, OP_TIMEOUT);
}

static int enable_ecc(void)
{
    feature_reg_configuration_t ecc_enable = {.whole = 0}; // we want to zero the other bits here
    ecc_enable.ECC_EN = 1;
    return set_feature(FEATURE_REG_CONFIGURATION, ecc_enable.whole, OP_TIMEOUT);
}

static int set_feature(uint8_t reg, uint8_t data, uint32_t timeout)
{
    // setup data
    uint8_t tx_data[FEATURE_TRANS_LEN] = {0};
    tx_data[0] = CMD_SET_FEATURE;
    tx_data[FEATURE_REG_INDEX] = reg;
    tx_data[FEATURE_DATA_INDEX] = data;
    // perform transaction
    csel_select();
    int ret = spi_write(tx_data, FEATURE_TRANS_LEN, timeout);
    csel_deselect();

    // map spi return to spi nand return
    return (SPI_RET_OK == ret) ? SPI_NAND_RET_OK : SPI_NAND_RET_BAD_SPI;
}

static int get_feature(uint8_t reg, uint8_t *data_out, uint32_t timeout)
{
    // setup data
    uint8_t tx_data[FEATURE_TRANS_LEN] = {0};
    uint8_t rx_data[FEATURE_TRANS_LEN] = {0};
    tx_data[0] = CMD_GET_FEATURE;
    tx_data[FEATURE_REG_INDEX] = reg;
    // perform transaction
    csel_select();
    int ret = spi_write_read(tx_data, rx_data, FEATURE_TRANS_LEN, timeout);
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

static bool validate_row_address(block_address_t block_address, page_address_t page_address)
{
    if ((block_address > SPI_NAND_MAX_BLOCK_ADDRESS) ||
        (page_address > SPI_NAND_MAX_PAGE_ADDRESS)) {
        return false;
    }
    else {
        return true;
    }
}

static bool validate_column_address(column_address_t address)
{
    if (address >= (SPI_NAND_PAGE_SIZE + SPI_NAND_OOB_SIZE)) {
        return false;
    }
    else {
        return true;
    }
}

static int poll_for_oip_clear(uint32_t timeout)
{
    uint32_t start_time = sys_time_get_ms();
    for (;;) {
        uint32_t get_feature_timeout = OP_TIMEOUT - sys_time_get_elapsed(start_time);
        feature_reg_status_t status;
        int ret = get_feature(FEATURE_REG_STATUS, &status.whole, get_feature_timeout);
        // break on bad return
        if (SPI_NAND_RET_OK != ret) {
            return ret;
        }
        // check for OIP clear
        if (0 == status.OIP) {
            return SPI_NAND_RET_OK;
        }
        // check for timeout
        if (sys_time_is_elapsed(start_time, timeout)) {
            return SPI_NAND_RET_TIMEOUT;
        }
    }
}

static int get_ecc_status(uint32_t timeout)
{
    feature_reg_status_t status = {.whole = 0};
    // get status reg
    int ret = get_feature(FEATURE_REG_STATUS, &status.whole, timeout);
    // exit on error
    if (SPI_NAND_RET_OK != ret) return ret;

    // map ECC status to return type
    switch (status.ECCS0_3) {
        case ECC_STATUS_NO_ERR:
        case ECC_STATUS_1_3_NO_REFRESH:
            ret = SPI_NAND_RET_OK;
            break;
        case ECC_STATUS_4_6_REFRESH:
        case ECC_STATUS_7_8_REFRESH:
            ret = SPI_NAND_RET_ECC_REFRESH;
            break;
        case ECC_STATUS_NOT_CORRECTED:
        default:
            ret = SPI_NAND_RET_ECC_ERR;
            break;
    }

    return ret;
}

static int write_enable(uint32_t timeout)
{
    // setup data
    uint8_t cmd = CMD_WRITE_ENABLE;
    // perform transaction
    csel_select();
    int ret = spi_write(&cmd, sizeof(cmd), timeout);
    csel_deselect();

    // map spi return to spi nand return
    return (SPI_RET_OK == ret) ? SPI_NAND_RET_OK : SPI_NAND_RET_BAD_SPI;
}