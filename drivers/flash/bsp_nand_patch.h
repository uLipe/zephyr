#ifndef __BSP_NAND_PATCH_H__
#define __BSP_NAND_PATCH_H__

#undef SPI_NOR_CMD_WRSR
#undef SPI_NOR_CMD_RDSR
#undef SPI_NOR_CMD_WRSR
#undef SPI_NOR_CMD_WRSR2
#undef SPI_NOR_CMD_RDSR2
#undef SPI_NOR_CMD_WRSR3
#undef SPI_NOR_CMD_RDSR3

#define SPI_NOR_CMD_SET_FEAT        0x1F    /* Write status register */
#define SPI_NOR_CMD_GET_FEAT        0x0F    /* Read status register */

/* Flash opcodes */
#define SPI_NOR_CMD_WRSR        0xC0    /* Write status register */
#define SPI_NOR_CMD_RDSR        0xC0    /* Read status register */
#define SPI_NOR_CMD_WRSR2       0xA0    /* Write status register 2 */
#define SPI_NOR_CMD_RDSR2       0xA0    /* Read status register 2 */
#define SPI_NOR_CMD_RDSR3       0xB0    /* Read status register 3 */
#define SPI_NOR_CMD_WRSR3       0xB0    /* Write status register 3 */

#undef SPI_NOR_CMD_RESET_EN
#undef SPI_NOR_CMD_RESET_MEM

#define SPI_NOR_CMD_RESET_EN    0xFF    /* Reset Enable */
#define SPI_NOR_CMD_RESET_MEM   0xFF    /* Reset Memory */


#undef SPI_NOR_CMD_SE
#define SPI_NOR_CMD_SE          0xD8    /* Sector erase */

#undef SPI_NOR_CMD_BULKE
#define SPI_NOR_CMD_BULKE       0xD8    /* Bulk Erase */

/* Page, sector, and block size are standard, not configurable. */
#undef SPI_NOR_PAGE_SIZE
#undef SPI_NOR_SECTOR_SIZE
#undef SPI_NOR_BLOCK_SIZE
#define SPI_NOR_PAGE_SIZE    2048
#define SPI_NOR_SECTOR_SIZE  SPI_NOR_PAGE_SIZE
#define SPI_NOR_BLOCK_SIZE   ((128 * 1024))

#endif /*__SPI_NOR_H__*/
