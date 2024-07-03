#include <stdio.h>
#include <stdlib.h>
#include "mathOperation.h"
#include "test.h"

#define TEST 1

int main(int argc, char** argv) 
{
    if(argc != 3)
    {
        fprintf(stderr, "Bad Input! Needs three arguments");
        return EXIT_FAILURE;
    }

    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    if (TEST) 
    {
        runTests();
        printf("ALL TESTS PASS\n");
    }

    printf("Sum: %d\n", addition(num1, num2));

    return EXIT_SUCCESS;
}
