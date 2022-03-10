/**
 * MIT License
 *
 * Copyright (c) 2021 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE
 */

#include "linux/spi/spi.h"
#include "linux/slab.h"

#include "tpm.h"
#include "tpm_tis_spi.h"
#include "tis_wrap.h"
#include "spi_wrap.h"

struct spi_device *spidev;
unsigned char data_buffer[TPM_BUFSIZE];
size_t response_length;
size_t offset;

extern int tpm_tis_spi_probe(struct spi_device *dev);
extern void tpm_tis_spi_remove(struct spi_device *dev);

int tis_init(void) {
    int rc = -1;
    spidev = kzalloc(sizeof(struct spi_device), GFP_KERNEL);
    struct device *dev = &spidev->dev;
    struct class *cls;
    tpm_class = kzalloc(sizeof(*cls), GFP_KERNEL);
    tpmrm_class = kzalloc(sizeof(*cls), GFP_KERNEL);

    if (!spidev)
        return -1;

    /* initialize SPI hardware layer */
    spi_init();

    /* initialize TIS layer */
    tpm_tis_spi_probe(spidev);

    rc = tpm_pm_resume(dev);
    if (rc != 0) {
        return -1;
    }

    return 0;
}

int tis_test(void) {
    struct device *dev = &spidev->dev;
    struct tpm_chip *chip = dev_get_drvdata(dev);
    unsigned char ba[10];
    int rc = -1;

    memset(ba, 0, sizeof(ba));

    rc = tpm_pm_resume(dev);
    if (rc != 0 && rc != 0x100) {
        return -1;
    } else if (rc == 0x100) {
        /**
         * TPM Error (0x100):
         * Error (2.0): TPM_RC_INITIALIZE
         * Description: TPM not initialized by TPM2_Startup or already initialized
         *
         * This is an expected error caused by tpm_pm_suspend() without actual power cycle.
         * Thus, TPM is still initialized.
         */
        //printf("Expected TPM Error 256 (0x100) due to no power cycle\r\n");
    }

    if (tpm_get_random(chip, ba, sizeof(ba)) < 0)
        return -1;

    /*printf("Get hardware random: Got %d bytes\r\n", sizeof(ba));
    for (int i=0;i<sizeof(ba);i++) {
        printf("%02X",ba[i]);
    }
    printf("\r\n");*/

    if (tpm_pm_suspend(dev))
        return -1;

    return 0;
}

void tis_release(void) {
    /* Release TIS layer */
    tpm_tis_spi_remove(spidev);
    kfree(spidev);

    kfree(tpm_class);
    kfree(tpmrm_class);

    /* Release SPI hardware layer */
    spi_release();
}

/**
 * should be protected by mutex
 * to support multithreading
 */
ssize_t tis_write(unsigned char *buf, size_t bufsiz) {
    struct device *dev = &spidev->dev;
    struct tpm_chip *chip = dev_get_drvdata(dev);
    struct tpm_space *space = &chip->work_space;
    size_t command_size = bufsiz;
    memcpy(data_buffer, buf, bufsiz);
    buf = data_buffer;
    bufsiz = sizeof(data_buffer);
    response_length = 0;

    struct tpm_header *header = (void *)buf;
    ssize_t ret, len;

    ret = tpm2_prepare_space(chip, space, buf, bufsiz);
    /* If the command is not implemented by the TPM, synthesize a
     * response with a TPM2_RC_COMMAND_CODE return for user-space.
     */
    if (ret == -EOPNOTSUPP) {
        header->length = cpu_to_be32(sizeof(*header));
        header->tag = cpu_to_be16(TPM2_ST_NO_SESSIONS);
        header->return_code = cpu_to_be32(TPM2_RC_COMMAND_CODE |
                          TSS2_RESMGR_TPM_RC_LAYER);
        ret = sizeof(*header);
    }
    if (ret)
        goto out_rc;

    /* request locality */
    if (tpm_try_get_ops(chip))
        return -EPIPE;

    len = tpm_transmit(chip, buf, bufsiz);
    if (len < 0)
        ret = len;
    else {
        response_length = len;
        offset = 0;
    }

    /* relinguish locality */
    //tpm_put_ops(chip);

    if (!ret)
        ret = tpm2_commit_space(chip, space, buf, &len);

out_rc:
    return ret ? ret : command_size;
}

/**
 * should be protected by mutex
 * to support multithreading
 */
ssize_t tis_read(unsigned char *buf, int size) {

    ssize_t ret;

    if (offset + size > response_length) {
        ret = -EIO;
        goto out;
    }

    ret = size;
    memcpy(buf, data_buffer + offset, ret);
    offset += ret;

    if (offset >= response_length) {
        response_length = 0;
        offset = 0;
        memset(data_buffer, 0, sizeof(data_buffer));
    }

out:
    return ret;
}
