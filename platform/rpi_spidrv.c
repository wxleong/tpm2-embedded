#include "spidrv.h"
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_NODE "/dev/spidev0.1"

static int fd;
struct spi_ioc_transfer xfer;

int spidrv_init()
{
    int ret = 0;
    unsigned int mode = ( SPI_MODE_0 );
    unsigned int speed = 100000; // 100kHz
    unsigned char bits = 8;


    fd = open(DEVICE_NODE, O_RDWR);
    if (fd < 0)
        return 1;

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
        return 1;

    ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1)
        return 1;

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        return 1;

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        return 1;

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        return 1;

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        return 1;

    memset(&xfer, 0, sizeof(xfer)); 
    xfer.bits_per_word = bits; 
    xfer.speed_hz = speed; 
    xfer.delay_usecs = 0;
    
    return 0;
}

int spidrv_xfer(const unsigned char *tx, const unsigned char *rx, size_t len, int cs_change)
{
    int ret;

    xfer.tx_buf = (unsigned long)tx;
    xfer.rx_buf = (unsigned long)rx;
    xfer.len = len;
    xfer.cs_change = cs_change; // 1: keep CS active after xfer, 0: deactivate CS after xfer

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
    if (ret < 1)
        return 1;

    return 0;
}

void spidrv_release()
{
    close(fd);
}
