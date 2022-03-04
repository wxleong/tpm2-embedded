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
extern int tis_write(const unsigned char *buf, int size);
extern int tis_read(unsigned char *buf, int size);

#endif /* TCTI_EMBEDDED_H */
