

// Author: Alexandar Dimitrov
// Student ID: 51769662
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "filesys.h"


int main() {
    format();
    // Declare a pointer to a pointer to char which will store the returned from the mylistdir function.
    char ** result;

    // Call mymkdir
    mymkdir("/firstdir/seconddir");

    // Call myfopen with a path and create a file
    MyFILE * file = myfopen("firstdir/seconddir/testfile1.txt", "w");
    if(!file) return 0;                                // file is being opened in a wrong mode; corresponding error message is shown

    
    
    // Insert text to the file
    char *content1 = "This is the first file content";
    int i;
    int counter = 0;
    for(i=0; i < strlen(content1); i++){                // loop through the string
        if(counter > strlen(content1)) {                // iterate through the string over and over again
            counter = 0;                                // reset the counter
        }
        myfputc(content1[counter], file);               // insert a single character at a time to the file
        counter++;
    }
    myfputc(EOF, file);                                 // insert EOF to end of the file
    printf("Write %d bytes to the file and close the file.\n", i);
    myfclose(file);                                     // close the file

    // Call mylisdir to list the content of a specific directory
    result = mylistdir("/firstdir/seconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    // Call mychdir to change currDirIndex (into an existing directory)
    mychdir("/firstdir/seconddir");

    // Call myfopen again (this time without a path)
    MyFILE * file2 = myfopen("testfile2.txt", "w");
    if(!file2) return 0;                                 // file is being opened in a wrong mode; corresponding error message is shown

    // Insert text to the file
    char *content2 = "This is the second file content";
    counter = 0; 
    for(i=0; i < strlen(content2); i++){                // loop through the string
        if(counter > strlen(content2)) {                // iterate through the string over and over again
            counter = 0;                                // reset the counter
        }
        myfputc(content2[counter], file2);              // insert a single character at a time to the file
        counter++;
    }
    myfputc(EOF, file2);                                // insert EOF to end of the file
    printf("Write %d bytes to the file and close the file.\n", i);
    myfclose(file2);                                    // close the file

    // Create a third directory
    mymkdir("/firstdir/seconddir/thirddir");


    // Call myfopen again
    MyFILE *file3 = myfopen("thirddir/testfile3.txt", "w");
    if(!file3) return 0;                                 // file is being opened in a wrong mode; corresponding error message is shown

    // Insert text to the file
    char *content3 = "This is the third file content";
    counter = 0; 
    for(i=0; i < strlen(content3); i++){                // loop through the string
        if(counter > strlen(content3)) {                // iterate through the string over and over again
            counter = 0;                                // reset the counter
        }
        myfputc(content3[counter], file3);              // insert a single character at a time to the file
        counter++;
    }
    myfputc(EOF, file3);                                // insert EOF to end of the file
    printf("Write %d bytes to the file and close the file.\n", i);
    myfclose(file3);                                    // close the file


    // Call mylisdir to list the content of a specific directory
    result = mylistdir("/firstdir/seconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }


    // Call mylisdir to list the content of a specific directory
    result = mylistdir("/firstdir/seconddir/thirddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    writedisk("virtualdiskA5_A1_a");

    printf("\n===========================================================================\n");

    // Cannot remove files with myrmdir
    myrmdir("/firstdir/seconddir/testfile1.txt");

    printf("\nMyremove():");
    myremove("/firstdir/seconddir/testfile1.txt");

    mychdir("/firstdir/seconddir");
    myremove("testfile2.txt");
    writedisk("virtualdiskA5_A1_b");        // write complete virtual disk to a file

    printf("\n===========================================================================\n");

    mychdir("thirddir");

    // Cannot remove folder with files inside.
    myrmdir("/firstdir/seconddir/thirddir");

    // Call mylisdir to list the content of the thirddir
    result = mylistdir("/firstdir/seconddir/thirddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    // Folder removal allowed
    myremove("testfile3.txt");
    writedisk("virtualdiskA5_A1_c");     // write complete virtual disk to a file

    printf("\n===========================================================================\n");
    mychdir("/firstdir/seconddir");

    // Call mylisdir to list the content of a specific directory
    result = mylistdir("/firstdir/seconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(result[i], "\0") != 0) {
            printf("---> ");
            puts(result[i]);
        }
    }

    myrmdir("thirddir");

    mychdir("/firstdir");

    // Cannot remove dirs with myremove()
    myremove("seconddir");

    myrmdir("/firstdir/seconddir");

    mychdir("/");
    myrmdir("firstdir");

    writedisk("virtualdiskA5_A1_d");        // write complete virtual disk to a file

    return 0;
}

