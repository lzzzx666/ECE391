#include "x86_desc.h"
#include "lib.h"
#include "../syscalls/ece391support.h"
#include "i8259.h"
#include "idt.h"
#include "page.h"
#include "rtc.h"
#include "debug.h"
#include "terminal.h"
#include "keyboard.h"
#define PASS 1
#define FAIL 0


/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $0x80");
}

/* Checkpoint 1 tests */


/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */

int idt_test()
{
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			printf("111");
			assertion_failure();
			result = FAIL;
		}
	}
	rtc_test();
	//test_terminal();
	return result;
}
/* exc_test
 * Inputs: vector: the exception vector number
 * Outputs: it will print the exception type and then freeze
 * Side Effects: None
 */
void exc_test(int vector)
{
	TEST_HEADER;
	int test1 = 0; // some variable used to test
	int test2 = 1;
	int test3;
	int *test4 = NULL;
	if (vector > 0x13 && vector!=0 && vector!=0x80  && vector!=0x21 && vector!=0x28){ //those are valid idt number
		printf("idt entry does not exist!");
		return;} // we will not use exception greater than 0x13
	else if (vector == 0)
	{ // the divide_error exception
		test3 = test2 / test1;
	}
	else if (vector == 14)
	{ // the page_fault exception
		test3 = *test4;
	}
	else
	{
		switch (vector)
		{ // other exception, use int to trigger
		case 1:
			asm volatile("int $1");
			break;
		case 2:
			asm volatile("int $2");
			break;
		case 3:
			asm volatile("int $3");
			break;
		case 4:
			asm volatile("int $4");
			break;
		case 5:
			asm volatile("int $5");
			break;
		case 6:
			asm volatile("int $6");
			break;
		case 7:
			asm volatile("int $7");
			break;
		case 8:
			asm volatile("int $8");
			break;
		case 9:
			asm volatile("int $9");
			break;
		case 10:
			asm volatile("int $10");
			break;
		case 11:
			asm volatile("int $11");
			break;
		case 12:
			asm volatile("int $12");
			break;
		case 13:
			asm volatile("int $13");
			break;
		case 15:
			asm volatile("int $15");
			break;
		case 16:
			asm volatile("int $16");
			break;
		case 17:
			asm volatile("int $17");
			break;
		case 18:
			asm volatile("int $18");
			break;
		case 19:
			asm volatile("int $19");
			break;
		case 0x80:
			asm volatile(
			"movl $0,%%eax\n\t" //0 is the system call parameter
			"int $0x80"
			::);
		};
	}
}

/**
 * Perform memory access tests based on the specified test vector.
 *
 * This function performs memory access tests to verify the accessibility of
 * different memory regions based on the provided test vector. It checks for
 * read access and returns a result code.
 *
 * @param vec - Test vector specifying the memory region to test:
 *   - 0: Valid video memory test.
 *   - 1: Valid kernel memory test.
 *   - 2: Invalid memory access (NULL pointer).
 *   - 3: Invalid memory access (before video memory).
 *   - 4: Invalid memory access (beyond video memory).
 *   - 5: Invalid memory access (beyond kernel memory).
 *   - 6: Invalid memory access (before kernel memory).
 *
 * @return PASS if memory access is successful for the specified test, FAIL otherwise.
 */
int page_test(int vec)
{
	TEST_HEADER;
	char test;
	int result = PASS;
	int i;
	switch (vec)
	{
	case 0:
		for (i = 0; i < 0x0FFF; i++) // vmem has 2^12 entries
		{
			test = *((char *)VIDEO + i);
		}
		TEST_OUTPUT("valid video memory test", result);
		break;
	case 1:
		for (i = 0; i < 4 * 1024 * 1024; i++) // kernel mem is 4mb = 4*1024*1024 bytes
		{
			test = *((char *)KERNEL_START_ADDR + i);
		}
		TEST_OUTPUT("valid kernel memory test", result);
		break;
	case 2:
		test = *((char *)NULL); // dereference NULL pointer
		result = FAIL;
		break;
	case 3:
		test = *((char *)VIDEO - 1); // 1 byte before vmem
		result = FAIL;
		break;
	case 4:
		test = *((char *)VIDEO + 0x1000); // 1 byte beyond vmem. 0x1000=0x0fff+1
		result = FAIL;
		break;
	case 5:
		test = *((char *)KERNEL_START_ADDR - 1); // 1 byte before kmem 4mb-1
		result = FAIL;
		break;
	case 6:
		test = *((char *)(8 * 1024 * 1024)); // 1 byte beyond kmem 8mb = 8*1024*1024
		result = FAIL;
		break;
	default:
		break;
	}

	return result;
}

/* Checkpoint 2 tests */
int test_terminal(){
	char buffer[128];
	memset((void*)buffer, 0, 128);
	int r = 0, w = 0;
	printf("terminal driver test begins\n");
	while (1)
	{
		r = terminal_read(0, buffer, 128);
		printf("read buf: %d\n", r);
		if(r >= 0)	w = terminal_write(0, buffer, 128);
		printf("read buf: %d, write buf:%d\n", r, w);
		if(r != w)
			break;
	}
	return -1;
}



/* test : change rtc freq */
int rtc_test() {
#ifdef RTC_VIRTUALIZE
	puts("==test RTC with virtualization==\n");
#else
	puts("==test RTC without virtualization==\n");
#endif
	uint32_t fd = rtc_open();
	uint32_t freq, j;
#ifdef RTC_VIRTUALIZE
	for (freq = 3; freq <= 50; freq += 3) {
#else
	for (freq = 2; freq <= INTERRUPT_FREQ_HI; freq <<= 1) {
#endif
		rtc_write(fd, &freq, sizeof(freq));
		printf("frequency: %d; ", freq);
		/*
		for(j = 0; j < 5 * freq; j++) {
			!rtc_read(fd, NULL, 0);
		}
		puts(" 5 sec"); 
		*/
		for(j = 0; j < 26; j++) {
			rtc_read(fd, NULL, 0);
			putc('A' + j);
		}
		putc('\n');
	}
	return !rtc_close(fd);
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{

	//TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("rtc_test", rtc_test());
	TEST_OUTPUT("test_terminal", test_terminal());
	// exc_test(0);
	// test_terminal();
}
