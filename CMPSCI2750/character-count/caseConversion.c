#include "caseConversion.h"


void toUppercase(char content[]) {
  for (int i = 0; i < strlen(content); i++) {
    char c = content[i];
    if (c >= LC_LOWER_BOUND && c <= LC_UPPER_BOUND) {
      content[i] = c - CASE_OFFSET;
    }
  }
}

void toLowercase(char content[]) {
  for (int i = 0; i < strlen(content); i++) {
    char c = content[i];
    if (c >= UC_LOWER_BOUND && c <= UC_UPPER_BOUND) {
      content[i] = c + CASE_OFFSET;
    }
  }
}
