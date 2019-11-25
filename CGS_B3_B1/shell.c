// Author: Alexandar Dimitrov
// Student ID: 51769662
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "filesys.h"


int main() {
    format();
    char ** result;

    printf("Call function mymkdir(\"/myfirstdir/myseconddir/mythirddir\"):\n");
    mymkdir("/myfirstdir/myseconddir/mythirddir");
    result = mylistdir("/myfirstdir/myseconddir");
    printf("Call function mylistdir(\"/myfirstdir/myseconddir\"):\n");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    writedisk("virtualdiskB3_B1_a");

    printf("\n===================================================================================================\n\n");

    printf("Call function mymkdir(\"/myfirstdir/myseconddir/testfile.txt\"):\n");
    mymkdir("/myfirstdir/myseconddir/testfile.txt");

    result = mylistdir("/myfirstdir/myseconddir");
    printf("Call function mylistdir(\"/myfirstdir/myseconddir\"):\n");

    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    writedisk("virtualdiskB3_B1_b");  //write complete virtual disk to a file
    return 0;
}