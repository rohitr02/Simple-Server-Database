#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#ifndef DEBUG
#define DEBUG true // CHANGE THIS BEFORE SUBMISSION
#endif

// check if a string has ONLY positive numbers -- checks for "12abcd" and "1abcd2" cases
int isLegalAtoiInput(char *arr) {
    for (char *ptr = arr; *ptr; ++ptr)
        if (!(*ptr <= 57 && *ptr >= 48)) // character is not btwn '0' and '9'
            return false;
    return true;
}