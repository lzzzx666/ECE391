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
#include "pcb.h"
#include  "systemcall.h"
#define PASS 1
#define FAIL 0
#define SAMPLE_NUMBER 10 //this is the size of the sample bytes umber
#define BUF_SIZE 100//this is the size of the whole buffer
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

/* test : change rtc freq */
void rtc_test()
{
#ifdef RTC_VIRTUALIZE
	puts("==RTC with virtualization==");
#else
	puts("==RTC no virtualization==");
#endif
	uint32_t fd = rtc_open((const uint8_t*)"rtc");
	uint32_t freq, j;
#ifdef RTC_VIRTUALIZE
	for (freq = 1; freq <= 20; freq += 2)
	{
#else
	for (freq = 2; freq <= INTERRUPT_FREQ_HI; freq <<= 1)
	{
#endif
		rtc_write(fd, &freq, sizeof(freq));
		printf("\nfrequency: %d; ... ", freq);
		/*
		for(j = 0; j < 5 * freq; j++) {
			!rtc_read(fd, NULL, 0);
		}
		puts(" 5 sec");
		*/
		for (j = 0; j < 26; j++)
		{
			rtc_read(fd, NULL, 0);
			putc('A' + j);
		}
	}
	rtc_close(fd);
}

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
	// test_terminal();
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
	if (vector > 0x13 && vector != 0 && vector != 0x80 && vector != 0x21 && vector != 0x28)
	{ // those are valid idt number
		printf("idt entry does not exist!");
		return;
	} // we will not use exception greater than 0x13
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
				"movl $0,%%eax\n\t" // 0 is the system call parameter
				"int $0x80" ::);
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
void test_terminal()
{
	char buffer[128];
	memset((void *)buffer, 0, 128);
	int r = 0, w = 0;
	printf("terminal driver test begins\n");
	while (1)
	{
		r = terminal_read(0, buffer, 128);
		printf("read buf: %d\n", r);
		if (r >= 0)
			w = terminal_write(0, buffer, 128);
		printf("read buf: %d, write buf:%d\n", r, w);
		if (r != w)
			break;
	}
	// return -1;
}

int filesys_test(int vec)
{
	TEST_HEADER;
	int result = PASS;
	switch (vec)
	{
	case 0:
		result = directory_read_test() == FS_SUCCEED;
		break;
	case 1:
		result = file_read_test("frame0.txt") == FS_SUCCEED;
		break;
	case 2:
		result = file_read_test("frame1.txt") == FS_SUCCEED;
		break;
	case 3:
		result = file_read_test("grep") == FS_SUCCEED;
		break;
	case 4:
		result = file_read_test("ls") == FS_SUCCEED;
		break;
	case 5:
		result = file_read_test("fish") == FS_SUCCEED;
		break;
	case 6:
		result = file_read_test("verylargetextwithverylongname.txt") == FS_SUCCEED;
		break;
	case 7:
		result = file_read_test("verylargetextwithverylongname.txt") == FS_FAIL;
		break;
	case 8:
		result = file_read_test("frame0.tx") == FS_FAIL;
		break;
	default:
		result = PASS;
	}
	return result;
}

/* Checkpoint 3 tests */
int sys_open_close_test(){
	TEST_HEADER;
	int result=PASS;
	/*the current pcb pointer*/
	pcb_t* current_pcb;
	/*some fds for different file types*/
	int reg_file,dir,rtc,garbage,try_open;
	/*the iterator*/
	int i;
	/*we use this to mimic a pcb*/
	create_pcb();
	current_pcb=get_current_pcb();
	/*test garbag open input*/
	printf("garbage open input test:\n");
	garbage=open((uint8_t*)"garbage");
	if(garbage!=-1){
		printf("garbage open input test fail!\n");
		result=FAIL;
	}else{
		printf("garbage open input test pass!\n");
	}
	/*then try regular file, dir, rtc*/
	printf("regular file, dir, rtc open input test:\n");
	reg_file=open((uint8_t*)"frame0.txt");
	dir=open((uint8_t*)".");
	rtc=open((uint8_t*)"rtc");
	if(current_pcb->file_obj_table[reg_file].exist && 
	current_pcb->file_obj_table[dir].exist &&
	current_pcb->file_obj_table[rtc].exist
	){
		printf("regular file, dir, rtc open input test pass!\n");
	}else{
		printf("regular file, dir, rtc open input test fail!\n");
		result=FAIL;
	}
	/*try to open more than 8 file*/
	printf("file capacity test:\n");
	for(i=0;i<8;i++){ //use 8 as a excessive file number
		open((uint8_t*)"frame0.txt");
	}
	if((try_open=open((uint8_t*)"frame0.txt"))!=-1){
		printf("file capacity test fail!\n");
		result=FAIL;
	}else{
		printf("file capacity test pass!\n");
	}
	/*delete excessive files*/
	for(i=0;i<8;i++){ //this 8 is the max file number
		if(i>rtc){
			close(i);
		}
	}
	/*fd filling test*/
	printf("file insert decision test:\n");
	i=reg_file;
	close(reg_file);
	reg_file=open((uint8_t*)"frame0.txt");
	if(reg_file!=i){
		printf("file insert decision test fail!\n");
		result=FAIL;
	}else{
		printf("file insert decision test pass!\n");
	}
	/*close garbage input test*/
	printf("garbage close input test:\n");
	if(close(-1)==-1){
	printf("garbage close input test pass!\n");
	}else{
	printf("garbage close input test fail!\n");
	result=FAIL;
	}
	/*reg,dir,rtc file close test*/
	printf("regular file, dir, rtc close test:\n");
	close(reg_file);
	close(dir);
	close(rtc);
	if(current_pcb->file_obj_table[reg_file].exist || 
	current_pcb->file_obj_table[dir].exist ||
	current_pcb->file_obj_table[rtc].exist
	){
		printf("regular file, dir, rtc close test fail!\n");
		result=FAIL;
	}else{
		printf("regular file, dir, rtc close test pass!\n");
	}
	/*close terminal test*/
	printf("close terminal test:\n");
	close(1);
	close(0);
	if(current_pcb->file_obj_table[0].exist && //0 and 1 are stdin/out
	current_pcb->file_obj_table[1].exist
	){
	printf("close terminal test pass!\n");
	}else{
	printf("close terminal test fail!\n");
	result=FAIL;
	}
	return result;

}
int sys_read_test(){
	TEST_HEADER;
	int result=PASS;
	/*the buf used to read*/
	uint8_t buf[BUF_SIZE]; //choose the buffer size to 100(maybe I don't use so much)
	/*the current pcb pointer*/
	pcb_t* current_pcb;
	/*define some different fds*/
	int file,dir,rtc,stdin=0,stdout=1;
	/*we use this to mimic a pcb*/
	create_pcb();
	current_pcb=get_current_pcb();
	/*first we test read*/
	/*open necessary file*/
	file=open((uint8_t*)"frame0.txt");
	rtc=open((uint8_t*)"rtc");
	dir=open((uint8_t*)".");
	/*test the regular file read*/
	printf("regular file read test:\n");
	memset(buf,'\0',BUF_SIZE);
	read(file,buf,SAMPLE_NUMBER); //just read first 10 bytes
	printf("the first 10 bytes of the file:\n");
	printf("\n%s\n",buf);

	/*test the regular file read*/
	printf("continue file read test:\n");
	read(file,buf,SAMPLE_NUMBER); //just read first 10 bytes
	printf("the second 10 bytes of the file:\n");
	printf("\n%s\n",buf);

	/*the dir read test*/
	/*I think ls program can test this so there is no need to add extra test*/
	memset(buf,'\0',BUF_SIZE);
	/*rtc read test*/
	printf("rtc read test:\n");
	if(read(rtc,buf,SAMPLE_NUMBER)!=0){
			printf("rtc read test fail!\n");
			result=FAIL;
	}else{
			printf("rtc read test pass!\n");
	}
	/*terminal read test*/
	printf("read in the stdout port test:\n");
	if(	read(stdout,buf,SAMPLE_NUMBER)!=-1){
		printf("read in the stdout port test fail!\n");
		result=FAIL;
	}else{
		printf("read in the stdout port test pass!\n");
	}
	printf("terminal read test:\n");
	memset(buf,'\0',BUF_SIZE);
	read(stdin,buf,SAMPLE_NUMBER);
	printf("print the context readed from stdin:\n");
	printf("%s\n",buf);

	return result;
}
int sys_write_test(){
	TEST_HEADER;
	int result=PASS;
	/*the buf used to write*/
	uint8_t buf[BUF_SIZE]; //choose the buffer size to 100(maybe I don't use so much)
	/*the current pcb pointer*/
	pcb_t* current_pcb;
	/*define some different fds*/
	int file,dir,rtc,stdin=0,stdout=1;
	/*some sample frequency*/
	int freq[2]={4,8};
	/*we use this to mimic a pcb*/
	create_pcb();
	current_pcb=get_current_pcb();
	/*open necessary file*/
	file=open((uint8_t*)"frame0.txt");
	rtc=open((uint8_t*)"rtc");
	dir=open((uint8_t*)".");
	/*write to the regular file*/
	memset(buf,'\0',SAMPLE_NUMBER);
	printf("write to the regular file test:\n");
	buf[0]='s';
	if(write(file,buf,SAMPLE_NUMBER)!=-1){
		printf("write to the regular file test fail!\n");
		result=FAIL;
	}else{
		printf("write to the regular file test pass!\n");
	}
	/*write to the dir*/
	memset(buf,'\0',BUF_SIZE);
	printf("write to the dir test:\n");
	buf[0]='s';
	if(write(dir,buf,100)!=-1){//use 100 to write the whole message
		printf("write to the dir test fail!\n");
		result=FAIL;
	}else{
		printf("write to the dir test pass!\n");
	}
	/*write to the terminal*/
	printf("write to the stdin(wrong port) test:\n");
	if(write(stdin,"any message",100)!=-1){//use 100 to write the whole message
		printf("write to the stdin test fail!\n");
		result=FAIL;
	}else{
		printf("write to the stdin test pass!\n");
	}
	memset(buf,'\0',BUF_SIZE);
	printf("write to the terminal test:\n");
	write(stdout,"The appear of this message indicates that write to terminal test pass!\n",100);//use 100 to write the whole message

	/*write to rtc test*/
	printf("write to the rtc test:\n");
	memset(buf,'\0',BUF_SIZE);
	write(rtc,"any message",sizeof(int32_t)); 

	return result;
}
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	// TEST_OUTPUT("sys_open_close_test",sys_open_close_test());
	TEST_OUTPUT("sys_read_test",sys_read_test());
	// TEST_OUTPUT("sys_write_test",sys_write_test());
}
