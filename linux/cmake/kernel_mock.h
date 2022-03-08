#ifndef KERNEL_MOCK_H
#define KERNEL_MOCK_H

extern void *malloc(size_t size);
extern void free(void *ptr);
extern int printf(const char *format, ...);

#endif
