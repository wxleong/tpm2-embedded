# for cross-compile
#set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)

add_compile_options(-Wall -nostdlib)

# kernel root dir
set(KERNEL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)

# kernel config is needed to prevent build errors
#set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../include/generated/autoconf.h)
set(KCONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../include/linux/kconfig.h)

# to minimize code change
# include kernel headers into the build
set(TPM2TIS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/char/tpm/)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
set(INCLUDE_UAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include/uapi/)
set(INCLUDE_ARCH_ARM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../arch/arm/include/)
set(INCLUDE_ARCH_ARM_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../arch/arm/include/generated/)
set(INCLUDE_ARCH_ARM_GEN_UAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../arch/arm/include/generated/uapi/)


set(TPM2TIS_FILES ${TPM2TIS_DIR}/tpm-interface.c
                  ${TPM2TIS_DIR}/tpm2-cmd.c
                  ${TPM2TIS_DIR}/tpm1-cmd.c
                  ${TPM2TIS_DIR}/tpm_tis_core.c
                  ${TPM2TIS_DIR}/tpm-chip.c
                  ${TPM2TIS_DIR}/tpm_tis_spi_main.c
                  ${TPM2TIS_DIR}/tpm2-space.c
                  ${CMAKE_CURRENT_SOURCE_DIR}/kernel_mock.c
                  ${CMAKE_CURRENT_SOURCE_DIR}/spi_wrap.c
                  ${CMAKE_CURRENT_SOURCE_DIR}/tis_wrap.c
                  ${KERNEL_ROOT_DIR}/crypto/hash_info.c
                  ${KERNEL_ROOT_DIR}/lib/string.c
                  ${KERNEL_ROOT_DIR}/lib/ctype.c
)

message(STATUS "TPM2TIS_FILES: ${TPM2TIS_FILES}")

add_library(tpm2tis STATIC
  ${TPM2TIS_FILES}
)

target_precompile_headers(tpm2tis
  PRIVATE
#  ${CONFIG_FILE}
  ${KCONFIG_FILE}
)

target_include_directories(tpm2tis
  PRIVATE
  ${TPM2TIS_DIR}
  ${INCLUDE_DIR}
  ${INCLUDE_UAPI_DIR}
  ${INCLUDE_ARCH_ARM_DIR}
  ${INCLUDE_ARCH_ARM_GEN_DIR}
  ${INCLUDE_ARCH_ARM_GEN_UAPI_DIR}
#  PUBLIC
#  ${TPM2TSS_ROOT_DIR}/include/tss2
)

target_compile_definitions(tpm2tis PUBLIC
  # the sole purpose of these configs are to pass compilation
  -D__KERNEL__
  -D__LINUX_ARM_ARCH__=6
  -DMODULE
)
