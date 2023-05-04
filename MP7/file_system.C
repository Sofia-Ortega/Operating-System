/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");

    disk = nullptr;
    size = 0;
    numOfInodes = 0;
    free_blocks = nullptr;

}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */

    disk->write(0, serializeInodes()); // inodes list saved
    disk->write(1, free_blocks); // free blocks saved


}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    disk = _disk;

    unsigned char* buf;
    disk->read(0, buf);

    deserializeInodes(buf);

    unsigned char buf2[SimpleDisk::BLOCK_SIZE];
    Console::puts("");
    disk->read(1, buf2);
    free_blocks = buf2;

    size = disk->size();

    Console::puts("[TEST] Mounting file - inodes: \n");
    for(int i = 0; i < MAX_INODES; i++) {
        assert(!inodes[i].free);
    }
    Console::puts("[PASSED]\n");


    /*
    Console::puts("[TEST] Mounting file - free_blocks: \n");
    for(int i = 2; i < SimpleDisk::BLOCK_SIZE; i++) {
        assert(free_blocks[i] == '0');
    }
    Console::puts("[PASSED]\n");

    */

    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    
    // serialize - write inode to sequence of bytes
    // deserialize - read inode to sequence of bytes


   unsigned char* buf;
   long end[1] = {0};
   buf = (unsigned char*) end;
   _disk->write(0, buf);


   // see bitmap implementaiton from prev PA
    unsigned char buf2[SimpleDisk::BLOCK_SIZE];
    buf2[0] = '1'; // inode list
    buf2[1] = '1'; // free list
    for(unsigned int i = 2; i < SimpleDisk::BLOCK_SIZE; i++) {
        buf2[i] = '0';
    }

    _disk->write(1, buf2);

    // ****** TESTING format of inode list***********
    
    Console::puts("[TEST] Format of block 0 - inode block \n");
    _disk->read(0, buf);
    long* l = (long*) buf;
    Console::puti((int) l[0]);
    assert(l[0] == 0);
    Console::puts("[PASSED]\n");

    // ****************** TESTING format of free block list *********
    
    Console::puts("[TEST] Format of block 1 - free block \n");
    _disk->read(1, buf2);
    assert(buf2[0] == '1' && buf2[1] == '1');
    for(unsigned int i = 2; i < SimpleDisk::BLOCK_SIZE; i++) {
        Console::putch(buf2[i]);
        Console::puts(", ");
        assert(buf2[i] == '0');
    }
    Console::puts("\n");
    Console::puts("[PASSED]\n");
    

    return true;

}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for(int i = 0; i < MAX_INODES; i++) {
        Inode in = inodes[i];
        if(!in.free && in.id == _file_id) return &inodes[i];
    }

    return nullptr;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    
    if(LookupFile(_file_id) != nullptr) {
        Console::puts("File id already exists - aborting\n");
        return false;
    }

    // get free block + inode
    int free_block = GetFreeBlock();
    int free_inode = GetFreeInode();

    // initialize inode at free_inode
    inodes[free_inode].id = _file_id;
    inodes[free_inode].block_no = free_block;
    inodes[free_inode].fileLength = 0;
    inodes[free_inode].free = false;
    inodes[free_inode].fs = nullptr;

    numOfInodes++;

    // set block as taken 
    free_blocks[free_block] = '1';

    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    
    Inode* in = LookupFile(_file_id);
    if (in == nullptr) {
        Console::puts("[ERROR] DeleteFile() - no inode found with id: "); Console::puti(_file_id); Console::puts("\n");
        return false;

    }

    // free up block no
    int blockNum = in->block_no;
    free_blocks[blockNum] = '0';

    // free up inode
    in->free = true;
    numOfInodes--;

    return true;

}

int FileSystem::GetFreeInode() {
    for(int i = 0; i < MAX_INODES; i++) {
        if(inodes[i].free) return i;
    }

    Console::puts("[ERROR] no free inode found\n");
    assert(false);
    return -1;
}

int FileSystem::GetFreeBlock() {
    for(int i = 0; i < SimpleDisk::BLOCK_SIZE; i++) {
        if(free_blocks[i] == '0') return i;
    }

    Console::puts("[ERROR] no free block found\n");
    assert(false);
    return -1;
}