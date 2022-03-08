CURRENT_ROOT_DIR := $(shell pwd)
TPM2TSS_ROOT_DIR := ${CURRENT_ROOT_DIR}/../../tpm2-tss
MBEDTLS_ROOT_DIR := ${CURRENT_ROOT_DIR}/../../mbedtls
TPM2TIS_ROOT_DIR := ${CURRENT_ROOT_DIR}/../../linux

SRC := $(wildcard *.c */*.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(SRC:.c=.d)

$(info Sources=$(SRC))
$(info Objects=$(OBJ))

CPPFLAGS := -MMD
CC := gcc
LDFLAGS :=
LDLIBS := -L${TPM2TSS_ROOT_DIR}/cmake/ -ltpm2tss -L${TPM2TIS_ROOT_DIR}/cmake/ -ltpm2tis -L${MBEDTLS_ROOT_DIR}/library/ -lmbedcrypto -lmbedtls -lmbedx509
CFLAGS := -Os -Wall -std=c99 -I${TPM2TSS_ROOT_DIR}/include/ -I${MBEDTLS_ROOT_DIR}/include/ -Itpm/ -I${TPM2TIS_ROOT_DIR}/cmake/

main: $(OBJ)

.PHONY: clean
clean:
	rm -f main $(OBJ) $(DEP)

-include $(DEP)
