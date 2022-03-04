set(TPM2TIS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../linux/cmake/)
set(TPM2TSS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(MBEDTLS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../mbedtls/)

file(GLOB_RECURSE TSS2_MU_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-mu/*.c)
file(GLOB_RECURSE TSS2_RC_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-rc/*.c)
file(GLOB_RECURSE TSS2_SYS_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-sys/*.c)
file(GLOB_RECURSE TSS2_ESYS_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-esys/*.c)
set(TSS2_TCTI_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-tcti/tcti-common.c
                    ${TPM2TSS_ROOT_DIR}/src/tss2-tcti/tctildr.c
                    ${TPM2TSS_ROOT_DIR}/src/tss2-tcti/tctildr-nodl.c
                    ${TPM2TSS_ROOT_DIR}/cmake/tcti-embedded.c
#                    ${TPM2TSS_ROOT_DIR}/src/tss2-tcti/tcti-mssim.c
#                    ${TPM2TSS_ROOT_DIR}/src/tss2-tcti/tcti-device.c
)
file(GLOB_RECURSE MBEDTLS_FILES ${MBEDTLS_ROOT_DIR}/library/*.a)
file(GLOB_RECURSE TPM2TIS_FILES ${TPM2TIS_ROOT_DIR}/*.a)

message(STATUS "TSS2_MU_FILES: ${TSS2_MU_FILES}")
message(STATUS "TSS2_RC_FILES: ${TSS2_RC_FILES}")
message(STATUS "TSS2_SYS_FILES: ${TSS2_SYS_FILES}")
message(STATUS "TSS2_ESYS_FILES: ${TSS2_ESYS_FILES}")
message(STATUS "TSS2_TCTI_FILES: ${TSS2_TCTI_FILES}")
message(STATUS "MBEDTLS_FILES: ${MBEDTLS_FILES}")

#list(REMOVE_ITEM TSS2_ESYS_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-esys/esys_crypto_mbed.c)
list(REMOVE_ITEM TSS2_ESYS_FILES ${TPM2TSS_ROOT_DIR}/src/tss2-esys/esys_crypto_ossl.c)

add_library(tpm2tss STATIC
  ${TSS2_RC_FILES}
  ${TSS2_SYS_FILES}
  ${TSS2_ESYS_FILES}
  ${TSS2_MU_FILES}
  ${TSS2_TCTI_FILES}
  ${TPM2TSS_ROOT_DIR}/src/util/io.c
  ${TPM2TSS_ROOT_DIR}/src/util/log.c
  ${TPM2TSS_ROOT_DIR}/src/util/key-value-parse.c
)

target_link_libraries(tpm2tss PRIVATE ${MBEDTLS_FILES})

target_include_directories(tpm2tss
  PRIVATE
  ${TPM2TSS_ROOT_DIR}/src
  ${TPM2TSS_ROOT_DIR}/src/tss2-sys
  ${TPM2TSS_ROOT_DIR}/src/tss2-esys
  ${MBEDTLS_ROOT_DIR}/include
  PUBLIC
  ${TPM2TSS_ROOT_DIR}/include/tss2
)

#target_compile_definitions(tpm2tss PUBLIC -DMAXLOGLEVEL=6 -DTCTI_DEVICE -DOSSL=1)
#target_compile_definitions(tpm2tss PUBLIC -DMAXLOGLEVEL=6 -DTCTI_DEVICE -DMBED=1)
#target_compile_definitions(tpm2tss PUBLIC -DMAXLOGLEVEL=6 -DTCTI_MSSIM -DMBED=1)
target_compile_definitions(tpm2tss PUBLIC -DMAXLOGLEVEL=6 -DTCTI_EMBEDDED -DMBED=1)