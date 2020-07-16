/**
 * @file		spi_nand.h
 * @author		Andrew Loebs
 * @brief		Header file of the spi nand module
 *
 * SPI NAND flash chip driver
 *
 */

#ifndef __SPI_NAND_H
#define __SPI_NAND_H

#include <stddef.h>
#include <stdint.h>

/// @brief SPI return statuses
#define SPI_NAND_RET_OK        0
#define SPI_NAND_RET_BAD_SPI   -1
#define SPI_NAND_RET_TIMEOUT   -2
#define SPI_NAND_RET_DEVICE_ID -3

/// @brief Initializes the spi nand driver
int spi_nand_init(void);

#endif // __SPI_NAND_H
