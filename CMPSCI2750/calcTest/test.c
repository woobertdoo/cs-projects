#include <stdio.h>
#include <stdlib.h>
#include "mathOperation.h"

void testAddition() 
{
    int a = 10;
    int b = 20;
    int expected = 30;

    int result = addition(a, b);

    if(expected != result)
    {
        fprintf(stderr,"testAddition has failed\n");
        fprintf(stderr, "actual: %d expected: %d\n", result, expected);
        exit(1);
    }
    return;
}

void runTests()
{
    testAddition();
}

