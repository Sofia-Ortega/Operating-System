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

    block_no = inod->block_no;
    fileLength = inod->fileLength;
    myInode = inod;

    // read file into block cache
    disk->read(block_no, block_cache);


}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    myInode->fileLength = fileLength;
    disk->write(block_no, block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");

    unsigned int counter = 0;
    for(unsigned int i = currPos; i < _n && !EoF(); i++) {
        _buf[counter++] = block_cache[currPos];
    }

    currPos += counter;

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
    Console::puts("checking for EoF\n");
    return currPos == fileLength;

}
