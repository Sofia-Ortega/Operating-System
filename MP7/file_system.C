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
    inodes = nullptr;
    free_blocks = nullptr;
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    assert(false);
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


    // FIXME



    disk->read(1, free_blocks);
    // FIXME

    size = disk->size();


}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    
    // serialize - write inode to sequence of bytes
    // deserialize - read inode to sequence of bytes



    
    /*
    unsigned char* buf;
    unsigned int counter = 0;
    for(unsigned int i = 0; i < MAX_INODES; i++) {
        buf[counter] = (unsigned char) (new Inode());
        counter += sizeof(Inode);
    }

    _disk->write(0, buf); // INODE block
    */

   unsigned char* buf;
   long end = -1;
   buf[0] = (unsigned char) end;
   _disk->write(0, buf);

   // see bitmap implementaiton from prev PA
    unsigned char* buf2;
    buf2[0] = '1'; // inode list
    buf2[1] = '1';
    for(unsigned int i = 2; i < SimpleDisk::BLOCK_SIZE; i++) {
        buf2[i] = '0';
    }

    _disk->write(1, buf2);

    return true;


    /***************** TEST SERIALIZATION **************
    Inode inod;
    Console::puts("[TEST] run serialization test: \n");
    for(long i = 0; i < 10; i++) {
        unsigned long j = i + 1;
        long* arr = inod.deserialize( inod.serialize(i, j) );
        Console::puts("["); Console::puti((int) i); Console::puts("] ");
        Console::puti((int) arr[0]); Console::puts(", "); Console::puti((int)arr[1]);
        Console::puts("\n");

        assert(i == arr[0]);
        assert(j == arr[1]);

    }

    Console::puts("[SUCCESS] serialization test passed \n");

    ****************************************************/


}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    // for(int i = 0; i < MAX_INODES; i++) {
    //     Inode in = inodes[i];
    //     if(!in.free && in.id == _file_id) return &inodes[i];
        
    // }
    Inode* curr = inodes;
    while(curr != nullptr) {
        if(curr->id == _file_id) return curr;
        curr = curr->next;
    }

    return nullptr;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    
    assert(LookupFile(_file_id) == nullptr);

    int block = GetFreeBlock();
    Inode new_inod;
    new_inod.id = _file_id;
    new_inod.fs = this;
    new_inod.block_no = block;

    // add to linked list
    new_inod.next = inodes;
    inodes = &new_inod;

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

    int blockNum = in->block_no;
    free_blocks[blockNum] = '0';


    Inode* curr = inodes;

    while(curr != nullptr) {
        if(curr->next->id == _file_id) {
            curr->next = curr->next->next;
            return true;
        };
        curr = curr->next;
    }

    Console::puts("[ERROR 2] DeleteFile() - no inode found with id: "); Console::puti(_file_id); Console::puts("\n");
    return false;

}

int FileSystem::GetFreeBlock() {
    for(int i = 0; i < SimpleDisk::BLOCK_SIZE; i++) {
        if(free_blocks[i] == '0') return i;
    }
}