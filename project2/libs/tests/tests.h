#ifndef _TESTS_H_
#define _TESTS_H_

/**
 * To run tests, include this file from main.c and call Test_Suite(...)
 * with the masks for the tests you would like to run,
 *    eg  Test_Suite(TEST_THING)                        // To test one thing
 *    or  Test_Suite(TEST_THING | TEST_OTHER_THING)     // To test multiple things
 *    or  Test_Suite(TEST_ALL)                          // To run all tests
 */

typedef enum {
    TEST_QUEUE          = 0x01,
    // TEST_THING       = 0x02,
    // TEST_OTHER_THING = 0x04,
    // TEST_NEXT_THING  = 0x08,
    TEST_ALL            = 0xFF // ie: TEST_QUEUE | TEST_THING | TEST_OTHER_THING | TEST_NEXT_THING ...
} TEST_MASKS;

/**
 * Runs all tests specified by the provided mask
 * Sets all pins to output low before testing
 * Raises PD0 while any tests are active
 * Raises PD1 while each specifed test is active
 */
void Test_Suite(TEST_MASKS);

#endif
