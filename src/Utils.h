#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>

#define PRINT_ERR(msg, ...) fprintf(stderr, "In %s (%d): " msg , __FILE__, __LINE__, ##__VA_ARGS__)


#endif