#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Offset between lowercase and uppercase characters in ASCII
#define CASE_OFFSET 32
// Lowercase ASCII bounds
#define LC_LOWER_BOUND 97
#define LC_UPPER_BOUND 122
// Uppercase ASCII bounds
#define UC_LOWER_BOUND 65
#define UC_UPPER_BOUND 90

// Converts `content` string to all uppercase letter `in place`
void toUppercase(char content[]);
// Converts `content` string to all lowercase letter `in place`
void toLowercase(char content[]);
