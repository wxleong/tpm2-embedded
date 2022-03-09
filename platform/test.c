#include <stdio.h>
#include "spidrv.h"

int main() {

    unsigned char rx[5];
    unsigned char tx[] = {0x80, 0xD4, 0, 0, 0};

    if (spidrv_init())
        printf("spidrv_init error\n");

    if (spidrv_xfer(tx, rx, 4, 1))
        printf("spidrv_xfer error\n");

    // expectation rx: 00 00 00 01
    printf("tx: %02x%02x%02x%02x rx: %02x%02x%02x%02x\n", tx[0],tx[1],tx[2],tx[3],rx[0],rx[1],rx[2],rx[3]);

    if (spidrv_xfer(tx+4, rx+4, 1, 0))
        printf("spidrv_xfer error\n");

    // expectation rx: 81
    printf("tx: %02x rx: %02x\n", tx[4],rx[4]);

    if (spidrv_xfer(tx, rx, 4, 1))
        printf("spidrv_xfer error\n");

    // expectation rx: 00 00 00 01
    printf("tx: %02x%02x%02x%02x rx: %02x%02x%02x%02x\n", tx[0],tx[1],tx[2],tx[3],rx[0],rx[1],rx[2],rx[3]);

    if (spidrv_xfer(tx+4, rx+4, 1, 0))
        printf("spidrv_xfer error\n");

    // expectation rx: 81
    printf("tx: %02x rx: %02x\n", tx[4],rx[4]);

    spidrv_release();

    return 0;
}
