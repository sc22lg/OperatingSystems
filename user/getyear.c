#include "kernel/types.h"
#include "user/user.h"
int main(void) {
    printf("Unix V6 was released in year %d, nearly 50 years ago!\n", getyear());
    exit(0);
}