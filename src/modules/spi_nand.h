/**
 * @file		spi_nand.h
 * @author		Andrew Loebs
 * @brief		Header file of the spi nand module
 *
 * SPI NAND flash chip driver for the Micron MT29F1G01ABAFDWB.
 *
 */

#ifndef __SPI_NAND_H
#define __SPI_NAND_H

#include <stddef.h>
#include <stdint.h>

/// @brief SPI return statuses
enum {
    SPI_NAND_RET_OK = 0,
    SPI_NAND_RET_BAD_SPI = -1,
    SPI_NAND_RET_TIMEOUT = -2,
    SPI_NAND_RET_DEVICE_ID = -3,
    SPI_NAND_RET_BAD_ADDRESS = -4,
    SPI_NAND_RET_BUFFER_LEN = -5,
    SPI_NAND_RET_ECC_REFRESH = -6,
    SPI_NAND_RET_ECC_ERR = -7,
};

#define SPI_NAND_MAX_BLOCK_ADDRESS 1023
#define SPI_NAND_MAX_PAGE_ADDRESS  63
#define SPI_NAND_PAGE_SIZE         2176 // 2k + 128
#define SPI_NAND_MAX_BYTE_ADDRESS  (SPI_NAND_PAGE_SIZE - 1)

/// @brief Nand block address (valid range 0-1023)
typedef uint16_t block_address_t;
/// @brief Nand page address (valid range 0-63)
typedef uint8_t page_address_t;
/// @brief Nand column address (valid range 0-2175)
typedef uint16_t column_address_t;

/// @brief Initializes the spi nand driver
int spi_nand_init(void);

/// @brief Performs a read page operation
int spi_nand_read_page(block_address_t block_address, page_address_t page_address,
                       column_address_t column_address, uint8_t *data_out, size_t data_out_len);

#endif // __SPI_NAND_H
