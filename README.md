# flash_management
(WIP) Flash management stack consisting of a flash translation layer ([dhara](https://github.com/dlbeer/dhara)) and an SPI NAND driver. Uses an ST NUCLEO-L432KC (STM32L432KCUX MCU) board connected to a Micron MT29F1G01ABAFDWB SPI NAND SLC flash chip.

## acknowledgements
- https://www.state-machine.com/quickstart/ - Miro Samek's "Modern Embedded Systems Programming Course". Great course for beginners, but also a great reference for writing your own linker and startup files.
- https://interrupt.memfault.com/blog/tag/zero-to-main - A series of blog posts detailing how to bootstrap C applications on cortex-m MCU's.
