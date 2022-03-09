#include "linux/spi/spi.h"
#include "linux/tpm.h"
#include "spi_wrap.h"
#include "kernel_mock.h"

int spi_sync_locked(struct spi_device *spi, struct spi_message *message)
{
    (void)spi;
    struct spi_transfer *xfer;
    unsigned char *tmp;
    int i;

    list_for_each_entry(xfer, &message->transfers, transfer_list) {

        if (xfer->tx_buf != NULL && xfer->rx_buf != NULL) {
            if (spidrv_xfer((const unsigned char *)xfer->tx_buf,
                            (const unsigned char *)xfer->rx_buf, xfer->len))
                return -EIO;
        }

        if (xfer->rx_buf != NULL) {
            tmp = malloc(xfer->len);
            for (i=0; i<xfer->len; i++) tmp[i] = 0;

            if (spidrv_xfer((const unsigned char *)tmp,
                            (const unsigned char *)xfer->rx_buf, xfer->len)) {
                free(tmp);
                return -EIO; 
            }

            free(tmp);
        }

    }

    return 0;
}

void spi_init()
{
    spidrv_init();
}

void spi_release()
{
    spidrv_release();
}
