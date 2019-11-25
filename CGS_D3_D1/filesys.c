/* filesys.c
*
* provides interface to virtual disk
* Author: Alexandar Dimitrov
* Student ID: 51769662
*/

// #include <unistd.h>
// #include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "filesys.h"


/* read and write FAT
*
* please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
*              our blocksize for the virtual disk is 1024, therefore
*              we can store 512 FAT entries in one block
*
*              how many disk blocks do we need to store the complete FAT:
*              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
*                each block is 1024 bytes long
*              - our FAT has MAXBLOCKS entries, which is currently 1024
*                each FAT entry is a fatentry_t, which is currently 2 bytes
*              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
*                FAT
*              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
*/

diskblock_t virtualDisk[MAXBLOCKS];           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t FAT[MAXBLOCKS];                   // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t rootDirIndex = 0;                 // rootDir will be set by format
direntry_t *currentDir = NULL;
fatentry_t currentDirIndex = 0;


// ------------------------------------------------------------------------------------------------------------------------------------
// D part functions

// initializeBlock() helper function
void initializeBlock(diskblock_t *block){
    for (int i = 0; i < BLOCKSIZE; i++) block->data[i] = '\0';
    // memset(block->data, '\0', BLOCKSIZE);   alternative
}

// copyFAT() helper function
void copyFAT(int fatblocksneeded) {
    // Copies the content of the FAT into one or more blocks (2 in my case),
    // then write these blocks to the virtual disk
    // Note: fatblocksneeded=2

    // Setting local variables
    diskblock_t block;
    for (int blockIndex=0, i=0; blockIndex < fatblocksneeded; blockIndex++) {
        for (int j = 0; j < FATENTRYCOUNT; j++, i++) {
            block.fat[j] = FAT[i];          // copying the FAT into the 2 needed blocks
        }
        // write the blocks to the virtual disk
        writeblock(&block, blockIndex+1);  // block 0 is reserved, so we need blocks 1 and 2 who are occupied by the FAT
    }
}

// implement format()
void format() {
    // Setting local variables
    diskblock_t block;
    diskblock_t rootDir;
    int fatentry = 0;
    int pos = 0;
    int fatblocksneeded = (MAXBLOCKS / FATENTRYCOUNT);  // 1024/512 = 2

    /* prepare block 0 : fill it with '\0',
     * use strcpy() to copy some text to it for testing purposes and
     * write block 0 to virtual disk
     */
    initializeBlock(&rootDir);      // initialize a rootDir block (set all its bytes to ‘\0’)
    initializeBlock(&block);        // initialize a disk block (set all its bytes to ‘\0’)
    strcpy(block.data, "CS3026 Operating Systems Assessment 2019");
    writeblock(&block, fatentry);   // writing to virtual disk


    /* prepare FAT table
     * write FAT blocks to virtual disk
     */
    // ---------------------------------------------------------------------------------------------------------------
    // Virtual Disk layout, i.e. known FAT layout:
    FAT[0] = ENDOFCHAIN;         // block 0 is reserved and can contain any info about the whole file system on the disk (e.g. volume name)
    FAT[1] = 2;
    FAT[2] = ENDOFCHAIN;        // blocks 1 and 2 are occupied by the FAT
    FAT[3] = ENDOFCHAIN;        // block 3 is the root directory
    //The rest of the virtual disk, blocks 4 – 1023, are either data or directory blocks:
    for (pos=4; pos < MAXBLOCKS; pos++) FAT[pos] = UNUSED;
    // ---------------------------------------------------------------------------------------------------------------
    copyFAT(fatblocksneeded);   // Copy FAT to Virtual Disk via a helper function


    /* prepare root directory
    * write root directory block to virtual disk
    */
    // A directory block has an array of directory entries (which can be files and directories)
    // and a “nextEntry” pointer that points to the next free list element for a directory entry
    rootDir.dir.isdir = TRUE;
    rootDir.dir.nextEntry = 0;      // set the first element in the entrylist
    writeblock(&rootDir, 3);        // block 3 in the Virtual Disk is the root directory
    rootDirIndex = 3;               // change rootDirIndex to index: fatblocksneeded+1
    currentDirIndex = 3;            // change the currentDirIndex as well
}
// ------------------------------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------------------------------
//pre-defined functions

void readdisk(const char *filename) {
    FILE *dest = fopen(filename, "r");
    if (fread(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
        fprintf(stderr, "write virtual disk to disk failed\n");
    //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
    fclose(dest);
}

/* writedisk : writes virtual disk out to physical disk
*
* in: file name of stored virtual disk
*/
void writedisk(const char *filename) {
    printf("writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data);
    FILE *dest = fopen(filename, "w");
    if (fwrite(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
        fprintf(stderr, "write virtual disk to disk failed\n");
    //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
    fclose(dest);
}

/*
* the basic interface to the virtual disk
* this moves memory around
*/
void writeblock(diskblock_t *block, int block_address) {
    //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
    memmove(virtualDisk[block_address].data, block->data, BLOCKSIZE);
    //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}

// use this for testing
void printBlock(int blockIndex) {
    printf("virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data);
}
// ------------------------------------------------------------------------------------------------------------------------------------