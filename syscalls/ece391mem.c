#include <stdint.h>

#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main()
{
    uint8_t buf[BUFSIZE];
    int length, i, j, size, digit,ptr;

    while (1)
    {
        length = 0;
        i = 0;
        size = 0;
        ptr=0;
        ece391_fdputs(1, "Free or allocate? f/a: ");
        ece391_read(0, buf, BUFSIZE);
        if (ece391_strncmp(buf, "f", 1) == 0) //the user chooses to allocate memory
        {
            ece391_fdputs(1, "Please enter the ptr: ");
            ece391_read(0, buf, BUFSIZE);
            while (buf[length] != '\0')
            {
                length++;
            }
            length--;
            /*calculate the ptr*/
            for (i = length - 1; i >= 0; i--)
            {
                digit = buf[i] - '0';
                for (j = 0; j < length - i - 1; j++)
                {
                    digit *= 10;
                }
                ptr += digit;
            }
            ece391_free((void*)ptr);
        }
        else if (ece391_strncmp(buf, "a", 1) == 0)  //the user chooses to free memory
        {
            ece391_fdputs(1, "Please enter the size: ");
            ece391_read(0, buf, BUFSIZE);
            while (buf[length] != '\0')
            {
                length++;
            }
            length--;
            /*calculate the size*/
            for (i = length - 1; i >= 0; i--)
            {
                digit = buf[i] - '0';
                for (j = 0; j < length - i - 1; j++)
                {
                    digit *= 10;
                }
                size += digit;
            }
            void *ptr = ece391_malloc(size);
        }
        else                                      //meaningless input
        {
            continue;
        }
    }

    return 0;
}
