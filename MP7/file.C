/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");

    disk = _fs->disk;

    Inode* inod = _fs->LookupFile(_id);

    assert(inod != nullptr);

    block_no = inod->block_no;
    fileLength = inod->fileLength;
    myInode = inod;

    currPos = 0;

    // read file into block cache
    disk->read(block_no, block_cache);


}

File::~File() {
    Console::puts("Closing file "); Console::puti(myInode->id); Console::puts("\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    myInode->fileLength = fileLength;
    Console::puts("CONTENTS OF FILE: "); Console::puti(fileLength); Console::puts("\n");
    for(int i = 0; i < fileLength; i++) {
        Console::putch((char) block_cache[i]);
    }
    Console::puts("\n");
    disk->write(block_no, block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");

    Console::puts("current position: "); Console::puti(currPos); Console::puts("\n");

    unsigned int counter = 0;
    for(; currPos < _n && currPos < fileLength; currPos++) {
        Console::putch((char) block_cache[currPos]);
        _buf[counter++] = block_cache[currPos];

    }

    Console::puts("\n");

    return counter;

}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    
    int counter = 0;
    for(; currPos < _n; currPos++) {
        if (currPos >= SimpleDisk::BLOCK_SIZE) break;
        block_cache[currPos] = _buf[counter++];
    }

    // update the fileLength 
    if(currPos > fileLength) fileLength = currPos;
    

    return counter;
}

void File::Reset() {
    Console::puts("resetting file\n");
    currPos = 0;
}

bool File::EoF() {
    // Console::puts("checking for EoF\n");
    return currPos == fileLength;

}
