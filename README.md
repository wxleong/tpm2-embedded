# Under Construction...

# Introduction

Enable OPTIGA™ TPM 2.0 on bare-metal / non-Linux embedded systems.

# Table of Contents

- **[Prerequisites](#prerequisites)**
- **[Decouple tpm2-tss Library](#decouple-tpm2-tss-library)**
- **[Decouple TIS/PTP Library](#decouple-tisptp-library)**
- **[Sample Application](#sample-application)**
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
 - bcm2835_defconfig: Raspberry Pi (32-bit ARM)
 - tinyconfig: 32-bit ARM
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

# Sample Application

```
$ cp -f ~/tpm2-embedded/tpm2-mbedtls/Makefile ~/tpm2-mbedtls/code/
$ cd ~/tpm2-mbedtls/code
$ make -j$(nproc)
$ ./main
```

# References

<a id="1">[1] https://www.infineon.com/cms/en/product/evaluation-boards/iridium9670-tpm2.0-linux/</a><br>
<a id="2">[2] https://github.com/wxleong/tpm2-rpi4</a><br>
<a id="3">[3] https://github.com/tpm2-software/tpm2-tss</a><br>
<a id="4">[4] https://github.com/wxleong/tpm2-mbedtls</a><br>
<a id="5">[5] https://github.com/wxleong/tpm2-embedded-linux</a><br>

# License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
