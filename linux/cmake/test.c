#include <stdio.h>
#include "tis_wrap.h"

int main() {

    int i;
    unsigned char buf[5];

    tis_init();

    if (tis_test(buf, sizeof(buf))) {
        printf("tis_test failed\n");
        return 1; 
    }

    printf("random bytes: ");

    for (i=0; i<sizeof(buf); i++)
        printf("%02x", buf[i]);

    printf("\n");

    //tis_release();

    return 0;
}
