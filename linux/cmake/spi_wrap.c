#include "linux/spi/spi.h"
#include "linux/tpm.h"

extern int spi_sync_locked(struct spi_device *spi, struct spi_message *message)
{
    /* SPI wrapper! */
}
