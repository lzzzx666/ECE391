#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
#define QUEUESIZE 16
#define UP 0xFE
#define DOWN 0xFF

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
	ece391_strcpy(cmdQueue.cmd[cmdQueue.tail], buf);
	cmdQueue.tail = (cmdQueue.tail + 1) % QUEUESIZE;
}

int main()
{
	int32_t cnt, rval, prevCnt, i;
	uint8_t buf[BUFSIZE];
	uint8_t prev[BUFSIZE];
	uint8_t hasPrev;
	cmdQueue.head = 0;
	cmdQueue.tail = 1;
	cmdQueue.curIdx = 0;

	ece391_fdputs(1, (uint8_t *)"Starting 391 Shell\n");

	while (1)
	{
		hasPrev = 0;
		ece391_fdputs(1, (uint8_t *)"391OS> ");
	next:
		if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
		{
			ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
			return 3;
		}
		if (UP == buf[cnt - 1])
		{
			if ((cmdQueue.curIdx == cmdQueue.head) || (cmdQueue.curIdx == (cmdQueue.head + 1) % QUEUESIZE))
			{
				ece391_fdputs(1, prev);
				goto next;
			}
			cmdQueue.curIdx = (cmdQueue.curIdx - 1 + QUEUESIZE) % QUEUESIZE;
			ece391_strcpy(prev, (const uint8_t *)cmdQueue.cmd[cmdQueue.curIdx]);
			ece391_fdputs(1, prev);
			hasPrev = 1;
			goto next;
		}
		if (DOWN == buf[cnt - 1])
		{
			if ((cmdQueue.curIdx == (cmdQueue.tail - 1 + QUEUESIZE) % QUEUESIZE) || (cmdQueue.curIdx == cmdQueue.tail))
			{
				ece391_fdputs(1, prev);
				goto next;
			}
			cmdQueue.curIdx = (cmdQueue.curIdx + 1) % QUEUESIZE;
			ece391_strcpy(prev, (const uint8_t *)cmdQueue.cmd[cmdQueue.curIdx]);
			ece391_fdputs(1, prev);
			hasPrev = 1;
			goto next;
		}
		if (cnt > 0 && '\n' == buf[cnt - 1])
		{
			if (cnt > 1)
			{
				buf[cnt - 1] = '\0';
				if (hasPrev == 0)
					push(buf);
			}
			cnt--;
		}
		cmdQueue.curIdx = cmdQueue.tail;
		if (0 == ece391_strcmp(buf, (uint8_t *)"exit"))
			return 0;
		if ('\0' == buf[0])
		{
			continue;
		}
		if (hasPrev != 0)
		{
			prevCnt = ece391_strlen(prev);
			for (i = 0; i < cnt; i++)
			{
				prev[prevCnt + i] = buf[i];
			}
			push(prev);
			cmdQueue.curIdx = cmdQueue.tail;
			rval = ece391_execute(prev);
			goto end;
		}
		rval = ece391_execute(buf);
	end:
		hasPrev = 0;
		if (-1 == rval)
			ece391_fdputs(1, (uint8_t *)"no such command\n");
		else if (256 == rval)
			ece391_fdputs(1, (uint8_t *)"program terminated by exception\n");
		else if (0 != rval)
			ece391_fdputs(1, (uint8_t *)"program terminated abnormally\n");
	}
}
