# SPI_BootLoader_FWDL

SPI BootLoarder for STM32 FWDL. Support C/C++ and python wrapper.


*The references are in the docs directory*
- *SPI protocol used in the STM32 bootloader.pdf*
- *AN2606-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf*

--- 
## Description

>The build.sh is to build the source code. Aim to generate libchip.so and test ELF file

## Usage

>build format:

```bash
	bash build.sh [release|clean]
```
	

> libchip.so will be built to "./build/release/lib" and executable test ELF file will be built to "./build/release/bin".eg:

```bash
	bash build.sh release
```

>clean format:

```bash
	bash build.sh clean
```

>command "build.sh clean" will delete the directory "build".


## Aplication
>1. HOST SPI connect to STM32 (NUCLEO STM32L452RE).

| HOST | STM32 |
| --- | --- |
| MOSI | SPI2_MOSI (PB15) |
| MISO | SPI2_MISO (PB14) |
| SCK | SPI2_SCK (PB13) |
| CS | SPI2_NSS (PB12) |

>2. STM32 BOOT0 pin to VDD, then power on.
>3. Refer to the test/main.cpp example to run the FWDL process
```C++
/* C++ example: test/main.cpp */
#include "DbgrCommon.h"
#include "stm32xxx.hpp"


char* bus_name = (char*)"/dev/AXI4_SPI_2";
char* bin_file = (char*)"/root/STM32CubeL4_Demo_STM32L452RE_Nucleo-V1.0.0.bin";
uint32_t flash_base_addr = 0x8000000;
uint32_t file_size = 37831;

STM32XXX stm32 = STM32XXX(spi_bus_name, 8000000, SpiCpol0nCpha0Mode, (char*)"/root/log/spi_bootloader.log");
stm32.init();
stm32.mass_erase();
stm32.program_only(bin_file, flash_base_addr, file_size);
stm32.verify(bin_file, flash_base_addr, file_size);
stm32.dump(bin_file, flash_base_addr, file_size);

```

```python
# python example: python_wrapper/spi_bootloader_fwdl.py
sbl_fwdl = SPIBootLoaderFWDL("/dev/AXI4_SPI_2", libchip="/root/lib/libchip.so", log_file="/root/log/spi_bootloader.log")
sbl_fwdl.init()
sbl_fwdl.mass_erase()
sbl_fwdl.program_only(bin_file, md5, flash_base_addr)
sbl_fwdl.verify(bin_file, md5, flash_base_addr)
sbl_fwdl.dump(bin_file, flash_base_addr, os.path.getsize(bin_file))
```