#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

int32_t main()
{

    int32_t fd = ece391_open((uint8_t *)"modex");

    return 0;
}
