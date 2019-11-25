/* filesys.c
*
* provides interface to virtual disk
* Author: Alexandar Dimitrov
* Student ID: 51769662
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>    // used for boolean values
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
    // Copies the content of the FAT into one or more blocks (2 in our case),
    // then write these blocks to the virtual disk
    // Note: fatblocksneeded=2

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
    copyFAT(fatblocksneeded);   // Copy FAT to Virtual Disk via a helper function
    // ---------------------------------------------------------------------------------------------------------------

    /* prepare root directory
    * write root directory block to virtual disk
    */
    // A directory block has an array of directory entries (which can be files and directories)
    // and a “nextEntry” pointer that points to the next free list element for a directory entry
    rootDir.dir.isdir = TRUE;
    // set every entrylist entry to unused since the directory is empty.
    for (int i = 0; i < DIRENTRYCOUNT; i++) rootDir.dir.entrylist[i].unused = TRUE;
    rootDir.dir.nextEntry = 0;      // set the first element in the entrylist
    writeblock(&rootDir, 3);        // block 3 in the Virtual Disk is the root directory
    rootDirIndex = 3;               // change rootDirIndex to index: fatblocksneeded+1
    currentDirIndex = 3;            // change the currentDirIndex as well
}
// ------------------------------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------------------------------
// C part functions
MyFILE * myfopen(const char * filename, const char * mode) {
    // Opens a file on the virtual disk and manages a buffer for it of size BLOCKSIZE,
    // mode may be either “r” for readonly or “w” for read/write/append (default “w”).
    // IF the file does not exist and the mode is 'w' then a new file is being created,
    // otherwise ('r' mode) False is returned.

    if (strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0){   // strcmp returns 0 if strings match
        printf("File mode not valid.\n");
        return FALSE;                                        // checking for correct mode
    }

    // Setting local variables
    int fatblocksneeded = (MAXBLOCKS / FATENTRYCOUNT);      // 1024/512 = 2
    bool existFile = false;                                 // boolean for checking if file already exists
    int pos;
    int freeListPos;
    diskblock_t block = virtualDisk[rootDirIndex];          // get directory entry block (block 4 at index 3 in the VD)

    MyFILE *file = malloc(sizeof(MyFILE));                  // allocating space for the file

    // =======================================================================================
    // Check for file existence in the directory block:
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        // The Loop will iterate 3 times at most
        if (block.dir.entrylist[i].unused) continue;
        if (strcmp(block.dir.entrylist[i].name, filename) == 0) {
            pos = i;
            existFile = true;
            break;
        }
    }
    // =======================================================================================

    // If file already exists
    if (existFile) {
        file->blockno = block.dir.entrylist[pos].firstblock;                // sets the file blockno being 1st block of the chain
        file->buffer  = virtualDisk[block.dir.entrylist[pos].firstblock];   // sets the buffer according to the virtual disk (the 1st block of the chain)
    }

    // If file does not exist and mode is write then a new file is being created, otherwise False is returned.
    else {
        // Read mode. False is returned.
        if (strcmp(mode, "r") == 0){
            printf("Nothing to read. File does not exist.\n");
            return FALSE;
        }

        // Write mode. File is being created.
        for (int i = 0; i < DIRENTRYCOUNT; i++) {                   // Look for empty directory entry
            if (block.dir.entrylist[i].unused) {
                freeListPos = i;
                break;
            }
        }

        // Looks for a free fat entry; starts from index 4 as first 4 blocks are already reserved
        for (pos = 4; pos < MAXBLOCKS; pos++) if (FAT[pos] == UNUSED) break;
        FAT[pos] = ENDOFCHAIN;
        copyFAT(fatblocksneeded);                                  // As stated each change in the FAT has to be copied to the Virtual Disk

        strcpy(block.dir.entrylist[freeListPos].name, filename);   // Set the filename to the operating block
        block.dir.entrylist[freeListPos].firstblock = pos;         // Set the position of first chain block in the directory list in accordance to the FAT table
        block.dir.entrylist[freeListPos].unused = FALSE;           // Set file to 'used'
        writeblock(&block, rootDirIndex);                          // Rewrite the root directory block to the virtual disk
        file->blockno = pos;                                       // Set block number of file to the 1st block of the chain
        file->buffer  = virtualDisk[pos];                          // Sets the buffer according to the virtual disk (the 1st block of the chain)

    }

    // File is being returned, set the mode and the starting position
    strcpy(file->mode, mode);                                       // Set the mode of the file
    file->pos = 0;                                                  // Set starting byte for file operations

    return file;
}


void myfclose(MyFILE * stream){
    // Closes the file, writes out any blocks not written to disk
    writeblock(&stream->buffer, stream->blockno);
    free(stream);
}


void myfputc(int b, MyFILE * stream) {
    // Used for WRITING.
    // Writes a byte to the file. Writes a block to the VD(Virtual Disk) when the buffer becomes full

    // Setting local variables
    int fatblocksneeded = (MAXBLOCKS / FATENTRYCOUNT);      // 1024/512 = 2
    int pos;

    diskblock_t * buffer = &stream->buffer;     // Get a pointer the the stream buffer (the start position of the file in fat)
    buffer->data[stream->pos] = b;              // Filling the data with content
    stream->pos++;                              // Incrementing the cursor pointer

    if (stream->pos >= BLOCKSIZE) {
        // When the buffer becomes full, the current buffer has to be written (copied) to the virtual disk and
        // a new block has to be allocated to the open file (an UNUSED block as indicated in the FAT)

        writeblock(buffer, stream->blockno);

        // Extending the Chain:
        for (pos = 4; pos < MAXBLOCKS; pos++) if (FAT[pos] == UNUSED) break;
        FAT[stream->blockno] = pos;             // continues the chain
        FAT[pos] = ENDOFCHAIN;                  // setting the end of the chain at this point of time

        stream->blockno = pos;                  // moving the blockno
        stream->pos = 0;                        // resetting the cursor position
        for(int i = 0; i < BLOCKSIZE; i++) buffer->data[i] = '\0';
    }

    copyFAT(fatblocksneeded);                   // As stated each change in the FAT has to be copied to the Virtual Disk
}


int myfgetc(MyFILE * stream) {
    // Used for READING.
    // Returns the next byte of the open file, or -1 (EOF == -1)
    char result;

    // The first block of the file has to be loaded (copied) from the virtual disk when opened;
    // Each read pushes a position pointer to the end of the buffer, when it becomes BUFFERSIZE then the next block
    // from the virtual disk has to be loaded;
    // For this the chain in the FAT has to be followed to find the next block of the file
    stream->buffer = virtualDisk[stream->blockno];
    result = stream->buffer.data[stream->pos];              // Getting the character byte
    stream->pos++;                                          // Incrementing the cursor

    // If read operations go beyond the current buffer content, the next buffer according to the block chain in the FAT has to be loaded.
    if (stream->pos >= BLOCKSIZE) {
        if (FAT[stream->blockno] >= ENDOFCHAIN) {           // End of chain is reached due to FAT (no more blocks to get)
            result = EOF;
        }
        stream->blockno = FAT[stream->blockno];             // Move blockNo to the next block in the FAT chain
        stream->pos = 0;                                    // Reset cursor
    }

    return result;                                          // Note: -1 could be returned due to end of the file content
}
// -----------------------------------------------------------------------------------------------------------------------------------


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
