#include <stdio.h>
#include <stdlib.h>

// Offset between lowercase and uppercase characters in ASCII
#define CASE_OFFSET 32

// Converts all alphabetic chars from the `fptr` input stream to uppercase and returns a `char` array of the contents
char* toUppercase(FILE *fptr);
// Converts all alphabetic chars from the `fptr` input stream to lowercase and returns a `char` array of the contents
char* toLowercase(FILE *fptr);
