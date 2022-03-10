/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2018 Intel Corporation
 * All rights reserved.
 */
#ifndef TCTI_EMBEDDED_H
#define TCTI_EMBEDDED_H

#include "tcti-common.h"

#define TCTI_EMBEDDED_MAGIC 0x3716704665907632ULL

typedef struct {
    TSS2_TCTI_COMMON_CONTEXT common;
} TSS2_TCTI_EMBEDDED_CONTEXT;

extern int tis_init(void);
extern void tis_release(void);
extern ssize_t tis_write(unsigned char *buf, size_t bufsiz);
extern ssize_t tis_read(unsigned char *buf, size_t bufsiz);

#endif /* TCTI_EMBEDDED_H */
