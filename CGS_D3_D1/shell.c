// Author: Alexandar Dimitrov
// Student ID: 51769662
#include <stdio.h>
#include <unistd.h>
#include "filesys.h"


int main() {
    format();
    writedisk("virtualdiskD3_D1");
    //    printBlock(0);
    return 0;
}