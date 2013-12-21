#include "utils.hh"

#include <string.h>
#include <stdio.h>

void int2str(std::string& rst_string, int int_val)
{
#define SIZE 12
    char buf[SIZE];
    memset(buf, sizeof buf, 0);
    snprintf(buf, sizeof buf, "%d", int_val);
    rst_string = buf;
}
