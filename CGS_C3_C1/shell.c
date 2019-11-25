// Author: Alexandar Dimitrov
// Student ID: 51769662
#include <stdio.h>
#include <unistd.h>
#include "filesys.h"


int main() {
    format();

    // =================================================================================================================
    // ****** WRITE TO FILE *******
    MyFILE * file = myfopen("testfile.txt", "w");
    if(!file) return 0;                                 // file is being opened in a wrong mode; corresponding error message is shown

    // Insert a text of size 4kb (4096 bytes) to the file
    char * string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int counter = 0;
    for(int i=0; i<(4*BLOCKSIZE); i++){                 // loop 4096 times
        if(counter>25) {                                // iterate through the alphabet over and over again (English Alphabet has 26 letters)
            counter = 0;                                // reset the counter
        }
        myfputc(string[counter], file);                 // insert a single character at a time to the file
        counter++;
    }
    myfputc(EOF, file);                                 // insert EOF to end of the file
    myfclose(file);                                     // close the file

                    // -------------------------------------------------
                    //    //TESTING (Writing a second file to the FAT):
                    //    MyFILE * test = myfopen("test.txt", "w");     // opening a second file for testing purposes
                    //    myfputc('+', test);                           // write a random symbol
                    //    myfputc(EOF, test);                           // insert EOF to end of the file
                    //    myfclose(test);                               // close the file
                    // ------------------------------------------------

    writedisk("virtualdiskC3_C1");                      // write the complete VD to a file 'virtualdiskC3_C1'
    // =================================================================================================================


    // =================================================================================================================
    // ***** READ FILE *******
    MyFILE *testFile = myfopen("testfile.txt", "w");
    FILE *resultFile = fopen("testfileC3_C1_copy.txt", "w");
    int character = myfgetc(testFile);
    while(character!= EOF){
        printf("%c", character);                     // testing
        fprintf(resultFile, "%c", character);
        character = myfgetc(testFile);
    }

    fclose(resultFile);
    myfclose(testFile);
    // =================================================================================================================

    return 0;
}