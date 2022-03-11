# Introduction

Enable OPTIGA™ TPM 2.0 on bare-metal / non-Linux embedded systems.

System overview: [architecture](docs/architecture.pptx)

# Table of Contents

- **[Prerequisites](#prerequisites)**
- **[Decouple tpm2-tss Library](#decouple-tpm2-tss-library)**
- **[Decouple TIS/PTP Library](#decouple-tisptp-library)**
- **[SPI Driver](#spi-driver)**
- **[Sample Application](#sample-application)**
- **[Miscellaneous](#miscellaneous)**
- **[References](#references)**
- **[License](#license)**

# Prerequisites

- Tested on Raspberry Pi 4 Model B with Iridium 9670 TPM 2.0 board [[1]](#1) 
- Set up the Raspberry Pi according to [[2]](#2) but skipping sections "Set Up TSS and Tools" and "Enable SPI TPM 2.0"
- Install dependencies:
    ```
    $ sudo apt install cmake git bc bison flex libssl-dev make
    ```
    <!-- $ sudo apt install cmake crossbuild-essential-armhf -->
- Download repos:
    ```
    $ git clone https://github.com/wxleong/tpm2-mbedtls ~/tpm2-mbedtls
    $ git clone https://github.com/wxleong/tpm2-embedded ~/tpm2-embedded
    ```

# Decouple tpm2-tss Library

```
$ git clone https://github.com/tpm2-software/tpm2-tss ~/tpm2-tss
$ cd ~/tpm2-tss
$ git checkout 3.2.0

$ cp -r ~/tpm2-embedded/tpm2-tss/cmake ~/tpm2-tss/
$ cd ~/tpm2-tss/cmake
$ rm -rf CMakeFiles/ CMakeCache.txt
$ cmake -j$(nproc) .
$ cmake --build . -j$(nproc)
```

# Decouple TIS/PTP Library

Download kernel source:
```
$ git clone --depth 1 --branch 1.20220120 https://github.com/raspberrypi/linux ~/linux
```

Edit the source according to:
- `~/tpm2-embedded/linux/patch/tpm_tis_spi_main.patch`
- `~/tpm2-embedded/linux/patch/tpm-interface.patch`

Generate the C headers:
```
$ cd ~/linux
$ make -j$(nproc) bcm2711_defconfig
$ make -j$(nproc) Image
$ ls arch/arm/include/generated
asm  uapi
```
<!--
No need to complete the "Image" build, headers are generated at the very beginning, CTRL+C to interrupt the build.
But to be safe, just complete the build...:')
-->
<!--
Cross-compile:
$ make -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2711_defconfig
$ make -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- Image
-->
<!--
Not all profiles will work, tested the following and NOT working:
 - multi_v7_defconfig: Beaglebone Black Wireless (32-bit ARM)
 - bcm2835_defconfig: Raspberry Pi (32-bit ARM, little endian)
 - sunxi_defconfig: 32-bit ARM, little or big endian
 - tinyconfig: 32-bit ARM
-->
<!--
for big endian, you will need to set compiler option "-mbig-endian", this will define __ARMEB__ and affecting arch/arm/include/asm/byteorder.h (this file only exist in older kernel e.g., 2.6, it is moved to compiler lib after). The byteorder.h will include linux/byteorder/big_endian.h or linux/byteorder/little_endian.h accordingly.

However, you cannot turn on "-mbig-endian" for any arch as you wish. Only arch that supports big endian can be built with the option without error. For instance, bcm2711_defconfig is for little endian only, if you enable the option, you will receive warning "inconsistent configuration, needs CONFIG_CPU_BIG_ENDIAN". Best is to check which arch will -> select ARCH_SUPPORTS_BIG_ENDIAN -> CONFIG_CPU_BIG_ENDIAN

arch that supports both big & small endian for 32-bit ARM i.e., sunxi_defconfig, remember to turn on big endian (System Type > Build big-endian kernel) & tpm & disable CONFIG_SUSPEND via menuconfig before building the kernel to generate C headers. remember to add "-mbig-endian" in tpm2tis.cmake when you are building the TIS library.

sunxi_defconfig tested to work on little endian too, remember to disable big endian (default disabled)(System Type > Build big-endian kernel) & enable tpm & disable CONFIG_SUSPEND via menuconfig before building the kernel to generate C headers. remember to remove "-mbig-endian" in tpm2tis.cmake when you are building the TIS library. sample-application runs ok on raspberry pi!
-->

Extract TIS library:
```
$ cp -r ~/tpm2-embedded/linux/cmake ~/linux/
$ cd ~/linux/cmake
$ rm -rf CMakeFiles/ CMakeCache.txt
$ cmake -j$(nproc) .
$ cmake --build . -j$(nproc)
```
<!-- Linux kernel .cofig file will be converted to ~/linux/include/generated/autoconf.h -->
<!-- autoconf.h is included in ~/linux/kconfig.h -->

# SPI Driver

The sample application runs on Raspberry Pi and therefore the SPI driver is using spidev to access the TPM:
```
$ cd ~/tpm2-embedded/platform
$ gcc -Wall -nostdlib -c rpi_spidrv.c -o rpi_spidrv.o
$ ar rcs libspidrv.a rpi_spidrv.o 
```

# Sample Application

Build the Mbed TLS static libraries according to [[6]](#6).

Build the sample application:
```
$ cp -f ~/tpm2-embedded/tpm2-mbedtls/Makefile ~/tpm2-mbedtls/code/
$ cd ~/tpm2-mbedtls/code
$ make -j$(nproc)
$ ./main
```

# Miscellaneous

Raspberry Pi SPI interface (spidev) testing:
```
$ cd ~/tpm2-embedded/platform
$ gcc -Wall test.c rpi_spidrv.c -o test
$ ./test
```

Linux TIS interface (tis_wrap) testing:
```
$ cd ~/linux/cmake
$ gcc -Wall test.c libtpm2tis.a ../../tpm2-embedded/platform/libspidrv.a -o test
$ ./test
```

Memory footprints collected from Raspberry Pi:
- libtpm2tss.a, 1702654 bytes
- libspidrv.a, 2252 bytes
- libtpm2tis.a, 134606 bytes

# References

<a id="1">[1] https://www.infineon.com/cms/en/product/evaluation-boards/iridium9670-tpm2.0-linux/</a><br>
<a id="2">[2] https://github.com/wxleong/tpm2-rpi4</a><br>
<a id="3">[3] https://github.com/tpm2-software/tpm2-tss</a><br>
<a id="4">[4] https://github.com/wxleong/tpm2-mbedtls</a><br>
<a id="5">[5] https://github.com/wxleong/tpm2-embedded-linux</a><br>
<a id="6">[6] https://github.com/wxleong/tpm2-mbedtls#mbed-tls-library<br>

# License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
