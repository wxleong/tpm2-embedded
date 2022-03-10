/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2015 - 2018 Intel Corporation
 * All rights reserved.
 * Copyright (c) 2019, Wind River Systems.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tss2_tcti.h"
#include "tss2_tcti_device.h"
#include "tss2_mu.h"
#include "tcti-common.h"
#include "tcti-embedded.h"
#define LOGMODULE tcti
#include "util/log.h"

TSS2_RC
tcti_device_transmit (
    TSS2_TCTI_CONTEXT *tctiContext,
    size_t command_size,
    const uint8_t *command_buffer)
{
    TSS2_TCTI_EMBEDDED_CONTEXT *tcti_dev = (TSS2_TCTI_EMBEDDED_CONTEXT *)tctiContext;
    TSS2_TCTI_COMMON_CONTEXT *tcti_common = &tcti_dev->common;
    TSS2_RC rc = TSS2_RC_SUCCESS;
    ssize_t size;

    rc = tcti_common_transmit_checks (tcti_common,
                                      command_buffer,
                                      TCTI_EMBEDDED_MAGIC);
    if (rc != TSS2_RC_SUCCESS) {
        return rc;
    }

    LOGBLOB_DEBUG (command_buffer,
                   command_size,
                   "sending %zu byte command buffer:",
                   command_size);
    size = tis_write((unsigned char *)command_buffer,
                      command_size);
    if (size < 0) {
        return TSS2_TCTI_RC_IO_ERROR;
    } else if ((size_t)size != command_size) {
        LOG_ERROR ("wrong number of bytes written. Expected %zu, wrote %zd.",
                   command_size,
                   size);
        return TSS2_TCTI_RC_IO_ERROR;
    }

    tcti_common->state = TCTI_STATE_RECEIVE;
    return TSS2_RC_SUCCESS;
}

TSS2_RC
tcti_device_receive (
    TSS2_TCTI_CONTEXT *tctiContext,
    size_t *response_size,
    uint8_t *response_buffer,
    int32_t timeout)
{
    TSS2_TCTI_EMBEDDED_CONTEXT *tcti_dev = (TSS2_TCTI_EMBEDDED_CONTEXT *)tctiContext;
    TSS2_TCTI_COMMON_CONTEXT *tcti_common = &tcti_dev->common;
    TSS2_RC rc = TSS2_RC_SUCCESS;
    ssize_t size = 0;
    uint8_t header[TPM_HEADER_SIZE];
    size_t offset = 2;
    UINT32 partial_size;

    rc = tcti_common_receive_checks (tcti_common,
                                     response_size,
                                     TCTI_EMBEDDED_MAGIC);
    if (rc != TSS2_RC_SUCCESS) {
        return rc;
    }

    if (!response_buffer) {
        if (!tcti_common->partial_read_supported) {
            LOG_DEBUG("Partial read not supported ");
            *response_size = 4096;
            return TSS2_RC_SUCCESS;
        } else {
            /* Read the header only and get the response size out of it */
            LOG_DEBUG("Partial read - reading response size");

            size = tis_read(header, TPM_HEADER_SIZE);
            if (size != TPM_HEADER_SIZE) {
                return TSS2_TCTI_RC_IO_ERROR;
            }

            LOG_DEBUG("Partial read - received header");
            rc = Tss2_MU_UINT32_Unmarshal(header, TPM_HEADER_SIZE,
                                          &offset, &partial_size);
            if (rc != TSS2_RC_SUCCESS) {
                LOG_ERROR ("Failed to unmarshal response size.");
                return rc;
            }
            if (partial_size < TPM_HEADER_SIZE) {
                LOG_ERROR ("Received %zu bytes, not enough to hold a TPM2"
               " response header.", size);
                return TSS2_TCTI_RC_GENERAL_FAILURE;
            }

            LOG_DEBUG("Partial read - received response size %d.", partial_size);
            tcti_common->partial = true;
            *response_size = partial_size;
            memcpy(&tcti_common->header, header, TPM_HEADER_SIZE);
            return rc;
        }
    }

    /* In case when the whole response is just the 10 bytes header
     * and we have read it already to get the size, we don't need
     * to call poll and read again. Just copy what we have read
     * and return.
     */
    if (tcti_common->partial == true && *response_size == TPM_HEADER_SIZE) {
        memcpy(response_buffer, &tcti_common->header, TPM_HEADER_SIZE);
        tcti_common->partial = false;
        goto out;
    }

    if (tcti_common->partial == true) {
        memcpy(response_buffer, &tcti_common->header, TPM_HEADER_SIZE);
        size = tis_read(response_buffer + TPM_HEADER_SIZE, *response_size - TPM_HEADER_SIZE);
    } else {
        size = tis_read(response_buffer, *response_size);
    }
    if (size < 0) {
        LOG_ERROR ("Failed to read response from tis_read, got errno %d: %s",
           errno, strerror (errno));
        return TSS2_TCTI_RC_IO_ERROR;
    }

    if (size == 0) {
        LOG_WARNING ("Got EOF instead of response.");
        rc = TSS2_TCTI_RC_NO_CONNECTION;
        goto out;
    }

    size += tcti_common->partial ? TPM_HEADER_SIZE : 0;
    LOGBLOB_DEBUG(response_buffer, size, "Response Received");
    tcti_common->partial = false;

    if ((size_t)size < TPM_HEADER_SIZE) {
        LOG_ERROR ("Received %zu bytes, not enough to hold a TPM2 response "
                   "header.", size);
        rc = TSS2_TCTI_RC_GENERAL_FAILURE;
        goto out;
    }

    rc = header_unmarshal (response_buffer, &tcti_common->header);
    if (rc != TSS2_RC_SUCCESS)
        goto out;

    LOG_DEBUG("Size from header %u bytes read %zu", tcti_common->header.size, size);

    if ((size_t)size != tcti_common->header.size) {
        LOG_WARNING ("TPM2 response size disagrees with number of bytes read "
                     "from tis_read. Header says %u but we read %zu bytes.",
                     tcti_common->header.size, size);
    }
    if (*response_size < tcti_common->header.size) {
        LOG_WARNING ("TPM2 response size is larger than the provided "
                     "buffer: future use of this TCTI will likely fail.");
        rc = TSS2_TCTI_RC_GENERAL_FAILURE;
    }
    *response_size = size;
    /*
     * Executing code beyond this point transitions the state machine to
     * TRANSMIT. Another call to this function will not be possible until
     * another command is sent to the TPM.
     */
out:
    tcti_common->state = TCTI_STATE_TRANSMIT;

    return rc;
}

void
tcti_device_finalize (
    TSS2_TCTI_CONTEXT *tctiContext)
{
    TSS2_TCTI_EMBEDDED_CONTEXT *tcti_dev = (TSS2_TCTI_EMBEDDED_CONTEXT *)tctiContext;
    TSS2_TCTI_COMMON_CONTEXT *tcti_common = &tcti_dev->common;

    if (tcti_dev == NULL) {
        return;
    }

    tis_release();

    tcti_common->state = TCTI_STATE_FINAL;
}

TSS2_RC
tcti_device_cancel (
    TSS2_TCTI_CONTEXT *tctiContext)
{
    UNUSED(tctiContext);
    return TSS2_TCTI_RC_NOT_IMPLEMENTED;
}

TSS2_RC
tcti_device_get_poll_handles (
    TSS2_TCTI_CONTEXT *tctiContext,
    TSS2_TCTI_POLL_HANDLE *handles,
    size_t *num_handles)
{
    (void) tctiContext;
    (void) handles;
    (void) num_handles;
    return TSS2_TCTI_RC_NOT_IMPLEMENTED;
}

TSS2_RC
tcti_device_set_locality (
    TSS2_TCTI_CONTEXT *tctiContext,
    uint8_t locality)
{
    UNUSED(tctiContext);
    UNUSED(locality);
    return TSS2_TCTI_RC_NOT_IMPLEMENTED;
}

TSS2_RC
Tss2_Tcti_Device_Init (
    TSS2_TCTI_CONTEXT *tctiContext,
    size_t *size,
    const char *conf)
{
    (void)(conf);

    if (tctiContext == NULL && size == NULL) {
        return TSS2_TCTI_RC_BAD_VALUE;
    } else if (tctiContext == NULL) {
        *size = sizeof (TSS2_TCTI_EMBEDDED_CONTEXT);
        return TSS2_RC_SUCCESS;
    }

    /* Init TCTI context */
    TSS2_TCTI_MAGIC (tctiContext) = TCTI_EMBEDDED_MAGIC;
    TSS2_TCTI_VERSION (tctiContext) = TCTI_VERSION;
    TSS2_TCTI_TRANSMIT (tctiContext) = tcti_device_transmit;
    TSS2_TCTI_RECEIVE (tctiContext) = tcti_device_receive;
    TSS2_TCTI_FINALIZE (tctiContext) = tcti_device_finalize;
    TSS2_TCTI_CANCEL (tctiContext) = tcti_device_cancel;
    TSS2_TCTI_GET_POLL_HANDLES (tctiContext) = tcti_device_get_poll_handles;
    TSS2_TCTI_SET_LOCALITY (tctiContext) = tcti_device_set_locality;
    TSS2_TCTI_MAKE_STICKY (tctiContext) = tcti_make_sticky_not_implemented;

    TSS2_TCTI_EMBEDDED_CONTEXT *tcti_dev = (TSS2_TCTI_EMBEDDED_CONTEXT *)tctiContext;
    TSS2_TCTI_COMMON_CONTEXT *tcti_common = &tcti_dev->common;
    tcti_common->state = TCTI_STATE_TRANSMIT;
    tcti_common->partial_read_supported = true;
    tcti_common->partial = false;

    if (tis_init())
        return TSS2_TCTI_RC_GENERAL_FAILURE;

    return TSS2_RC_SUCCESS;
}

const TSS2_TCTI_INFO tss2_tcti_info = {
    .version = TCTI_VERSION,
    .name = "tcti-embedded",
    .description = "TCTI module for communication with embedded interface.",
    .config_help = "",
    .init = Tss2_Tcti_Device_Init,
};

const TSS2_TCTI_INFO*
Tss2_Tcti_Info (void)
{
    return &tss2_tcti_info;
}
