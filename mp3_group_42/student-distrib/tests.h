#ifndef TESTS_H
#define TESTS_H

// test launcher
void launch_tests();
/*exception test*/
void exc_test(int vector);
/*open and close test*/
int sys_open_close_test();
/*read test for all kinds of file*/
int sys_read_test();
/*write test for all kinds of file*/
int sys_write_test();

#endif /* TESTS_H */
