

/* This is auto-generated code. Edit at your own peril. */

#include <stdlib.h>
#include <stdio.h>

#include "CuTest.h"


extern void Test_critbit_basics(CuTest*);


int RunAllTests(void) 
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();


    SUITE_ADD_TEST(suite, Test_critbit_basics);

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return (suite->failCount > 0) ? suite->failCount : 0;
}

int main(void)
{
    return RunAllTests();
}

