#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "../syscalls/ece391support.h"
#include "i8259.h"
#include "idt.h"
#include "idt_linkage.h"
#include "page.h"
#include "rtc.h"
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

int idt_test(){
	
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}
/* exc_test
 * Inputs: vector: the exception vector number
 * Outputs: it will print the exception type and then freeze
 * Side Effects: None
 */
void exc_test(int vector){
	int test1=0; //some variable used to test
	int test2=1;
	int test3;
	int *test4=NULL;
	if(vector>0x13) return; //we will not use exception greater than 0x13
	else if(vector==0){     //the divide_error exception
		test3=test2/test1;
	}else if(vector==14){   //the page_fault exception
		test3=*test4;
	}else{
		switch(vector){		//other exception, use int to trigger
			case 1:asm volatile("int $1");
			case 2:asm volatile("int $2");
			case 3:asm volatile("int $3");
			case 4:asm volatile("int $4");
			case 5:asm volatile("int $5");
			case 6:asm volatile("int $6");
			case 7:asm volatile("int $7");
			case 8:asm volatile("int $8");
			case 9:asm volatile("int $9");
			case 10:asm volatile("int $10");
			case 11:asm volatile("int $11");
			case 12:asm volatile("int $12");
			case 13:asm volatile("int $13");
			case 15:asm volatile("int $15");
			case 16:asm volatile("int $16");
			case 17:asm volatile("int $17");
			case 18:asm volatile("int $18");
			case 19:asm volatile("int $19");
		};

	}
}

// add more tests here

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	exc_test(1);
	TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
}
