/*

**to build user programs, run `gen.sh` in `/syscalls`.**

to-do list

- [x] UP+TAB
- Ctrl+L
*/
#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
#define QUEUESIZE 16

#define MAX_MATCH 64
#define SBUFSIZE 33

uint8_t matches[MAX_MATCH][SBUFSIZE];
uint8_t noMatches = 0;
uint8_t restore_last = 0;
int32_t last_pos = -1;

int32_t last_space_pos(const uint8_t *str, uint32_t len)
{
    int32_t i;
    for (i = len - 1; i >= 0; i--)
        if (str[i] == ' ')
            return i;
    return -1;
}

uint32_t get_match_list(const uint8_t *cmd, uint32_t len)
{
    int32_t fd, cnt;
    static uint8_t buffer[SBUFSIZE];
    if (-1 == (fd = ece391_open((uint8_t *)".")))
    {
        ece391_fdputs(1, (uint8_t *)"directory open failed\n");
        return 2;
    }
    uint32_t ret = 0;
    while (0 != (cnt = ece391_read(fd, buffer, SBUFSIZE - 1)))
    {
        if (-1 == cnt)
        {
            ece391_fdputs(1, (uint8_t *)"directory entry read failed\n");
            ret = 0;
            break;
        }
        buffer[cnt] = '\0';
        uint32_t match_from = ece391_strlen(cmd);
        if (ece391_strncmp(cmd, buffer, match_from) == 0)
        {
            ece391_strcpy(matches[ret], buffer);
            ret++;
        }
    }
    ece391_close(fd);
    return ret;
}

void do_tab(uint8_t *cmd, uint32_t len)
{
    restore_last = 0;
    noMatches = 0;
    uint32_t num_match = get_match_list(cmd, len);
    uint32_t i;
    if (num_match == 1)
    {
        ece391_simulate_keyboard();
        ece391_fdputs(1, matches[0]);
        return;
    }
    else if (num_match > 1)
    {
        ece391_fdputs(1, cmd);
        ece391_fdputs(1, (uint8_t *)"\n");
        for (i = 0; i < num_match; i++)
        {
            ece391_fdputs(1, matches[i]);
            ece391_fdputs(1, (uint8_t *)"  ");
        }
        restore_last = 1;
        ece391_fdputs(1, (uint8_t *)"\n");
    }
    else
    {
        noMatches = 1;
        ece391_simulate_keyboard();
        ece391_fdputs(1, cmd);
    }
}

// ====================================

typedef struct queue
{
    uint8_t cmd[QUEUESIZE][128];
    uint8_t head;
    uint8_t tail;
    uint8_t curIdx;
} queue_t;

queue_t cmdQueue;

void push(const uint8_t *buf)
{
    if (cmdQueue.head == (cmdQueue.tail + 1) % QUEUESIZE)
        cmdQueue.head = (cmdQueue.head + 1) % QUEUESIZE;
    cmdQueue.tail = (cmdQueue.tail + 1) % QUEUESIZE;
    ece391_strcpy(cmdQueue.cmd[cmdQueue.tail], buf);
    cmdQueue.curIdx = (cmdQueue.tail + 1) % QUEUESIZE;
}

int main()
{
    int32_t cnt, rval, i;

    uint8_t buf[BUFSIZE];
    uint8_t firstCmd[128];
    cmdQueue.head = 0;
    cmdQueue.tail = 0;
    cmdQueue.curIdx = 0;

    ece391_fdputs(1, (uint8_t *)"Starting 391 Shell\n");
    while (1)
    {
        ece391_fdputs(1, (uint8_t *)"391OS> ");
    next:
        last_pos = -1;
        if (restore_last)
        {
            ece391_simulate_keyboard();
            ece391_fdputs(1, buf);
        }
        if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
        {
            ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
            return 3;
        }

        if (cnt == 0)
            continue;
        if (ece391_strcmp((const uint8_t *)buf, (const uint8_t *)"^[[A") == 0)
        {
            if (cmdQueue.curIdx != (cmdQueue.head + 1) % QUEUESIZE)
                cmdQueue.curIdx = (cmdQueue.curIdx - 1 + QUEUESIZE) % QUEUESIZE;
            if (cmdQueue.head == 0 && cmdQueue.tail == 0)
                goto next;
            ece391_simulate_keyboard();
            ece391_fdputs(1, cmdQueue.cmd[cmdQueue.curIdx]);
            goto next;
        }
        if (ece391_strcmp((const uint8_t *)buf, (const uint8_t *)"^[[B") == 0)
        {
            if (cmdQueue.curIdx != cmdQueue.tail)
                cmdQueue.curIdx = (cmdQueue.curIdx + 1) % QUEUESIZE;
            if (cmdQueue.head == 0 && cmdQueue.tail == 0)
                goto next;
            ece391_simulate_keyboard();
            ece391_fdputs(1, cmdQueue.cmd[cmdQueue.curIdx]);
            goto next;
        }

        if (buf[cnt - 1] == '\t')
        {
            cnt--;
            buf[cnt] = '\0';
            last_pos = last_space_pos(buf, cnt);
            if (last_pos != -1)
            {
                ece391_strcpy(firstCmd, buf);
                firstCmd[last_pos + 1] = '\0';
                ece391_simulate_keyboard();
                ece391_fdputs(1, firstCmd);
                do_tab(&(buf[last_pos + 1]), cnt - last_pos);
            }
            else
                do_tab(buf, cnt);
            if (cnt == 0 || noMatches)
                restore_last = 0;
            if (cnt == 0 || restore_last == 1)
                continue;
            goto next;
        }
        restore_last = 0;

        if (cnt > 0)
        {
            buf[cnt - 1] = '\0';
            if (cnt > 1)
            {
                push(buf);
            }
        }

        if (0 == ece391_strcmp(buf, (uint8_t *)"exit"))
            return 0;

        if ('\0' == buf[0])
            continue;

        rval = ece391_execute(buf);

        if (-1 == rval)
        {
            ece391_fdputs(1, buf);
            ece391_fdputs(1, (uint8_t *)": no such command\n");
        }
        else if (256 == rval)
            ece391_fdputs(1, (uint8_t *)"program terminated by exception\n");
        else if (0 != rval)
            ece391_fdputs(1, (uint8_t *)"program terminated abnormally\n");
        for (i = 0; i < BUFSIZE; i++)
            buf[i] = 0;
    }
}
