#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
#define QUEUESIZE 16
#define UP 0xFE
#define DOWN 0xFF

#define MAX_MATCH 64
#define SBUFSIZE 33

uint8_t is_prefix(const uint8_t *str, const uint8_t *prefix, uint32_t prefix_len) {
	uint32_t i;
	for(i = 0; i < prefix_len; i++) {
		if(prefix[i] == '\0') return 1;
		if(str[i] == '\0') return 0;
		if(str[i] != prefix[i]) return 0;
	}
	return 1;
}

uint8_t matches[MAX_MATCH][SBUFSIZE];
uint8_t buf[BUFSIZE];
uint8_t last_pos = 0;
uint8_t restore_last = 0;

uint32_t get_match_list(const uint8_t *cmd, uint32_t len) {

	int32_t fd, cnt;
    static uint8_t buffer[SBUFSIZE];

    if (-1 == (fd = ece391_open ((uint8_t*)"."))) {
        ece391_fdputs (1, (uint8_t*)"directory open failed\n");
        return 2;
    }

	uint32_t ret = 0;
    while (0 != (cnt = ece391_read (fd, buffer, SBUFSIZE-1))) {
        if (-1 == cnt) {
	        ece391_fdputs (1, (uint8_t*)"directory entry read failed\n");
			ret = 0;
			break;
	    }
	    buffer[cnt] = '\0';
		if(is_prefix(cmd, buffer, len)) {
			ece391_strcpy(matches[ret++], buffer);
		}     
    }
	ece391_close(fd);
	return ret;
}

void do_tab(const uint8_t *cmd, uint32_t len) {
	restore_last = 0;
	uint32_t num_match = get_match_list(cmd, len);
	uint32_t i;
	if(num_match == 1) {
		ece391_fdputs(1, matches[0] + len);
		ece391_strcpy(buf, matches[0]);
		last_pos = ece391_strlen(buf);
		return;
	}
 	else if(num_match > 1) {
		ece391_fdputs(1, "\n");
		for(i = 0; i < num_match; i++) {
			ece391_fdputs(1, matches[i]);
			ece391_fdputs(1, "  ");
		}
		restore_last = 1;
		last_pos = len;
 	}
	else {	// no match
		last_pos = 0;
	}
	ece391_fdputs(1, "\n");
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
	ece391_strcpy(cmdQueue.cmd[cmdQueue.tail], buf);
	cmdQueue.tail = (cmdQueue.tail + 1) % QUEUESIZE;
}

int main ()
{
    int32_t cnt, rval, fd;
	int32_t tab_pressed = 0;

	int32_t prevCnt, i;
	uint8_t buf[BUFSIZE];
	uint8_t prev[BUFSIZE];
	uint8_t hasPrev;
	cmdQueue.head = 0;
	cmdQueue.tail = 1;
	cmdQueue.curIdx = 0;

    ece391_fdputs (1, (uint8_t*)"Starting 391 Shell\n");

    while (1) {
	hasPrev = 0;
	if(!last_pos || restore_last)
		ece391_fdputs (1, (uint8_t*)"391OS> ");
	next:
	if(restore_last) ece391_fdputs (1, buf);
	if (-1 == (cnt = ece391_read (0, buf + last_pos, BUFSIZE-1))) {
	    ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
	    return 3;
	}
	cnt += last_pos;
	if(cnt == 0) continue;

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

	if(buf[cnt - 1] == '\t') {
		cnt--;
		buf[cnt] = '\0';
		do_tab(buf, cnt);
		continue;
	}
	else if (cnt > 0 && '\n' == buf[cnt - 1])
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
	    continue;
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
	rval = ece391_execute (buf);

	if (-1 == rval) {
	    ece391_fdputs (1, buf);
	    ece391_fdputs (1, (uint8_t*)": no such command\n");
	}
	else if (256 == rval)
	    ece391_fdputs (1, (uint8_t*)"program terminated by exception\n");
	else if (0 != rval)
	    ece391_fdputs (1, (uint8_t*)"program terminated abnormally\n");
    }
}
