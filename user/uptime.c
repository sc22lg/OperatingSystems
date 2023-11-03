#include "kernel/types.h"
#include "user/user.h"


int main(void){

    int ut = uptime();
    printf("uptime: %d ticks\n", ut);

    exit(0);
}
