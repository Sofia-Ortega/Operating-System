/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "machine.H"
#include "common.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
BlockingDisk* BlockingDisk::blockedQueue = nullptr;
BlockingDisk* BlockingDisk::tail = nullptr;

void BlockingDisk::push_queue() {
  if (blockedQueue == nullptr) {
    blockedQueue = this;
  } else {
    // push to end of queue 
    tail->next = this; 
    tail = this;
  }
}

BlockingDisk* BlockingDisk::pop_queue() {
  assert(blockedQueue != nullptr); // make sure SOMETHING in queue

  // is head of blocked queue ready?
    // yes -> pop from queue and return
    // no -> return nullptr (fail)
  
  BlockingDisk* disk = blockedQueue;
  if(disk->is_ready()) {
    // pop from queue
    blockedQueue = blockedQueue->next;
    return disk;
  } 

  return nullptr;


  
}

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    next = nullptr;
    
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

  issue_operation(DISK_OPERATION::READ, _block_no);

  while (!is_ready()) {
    bool disk = true;
    Console::puts("\nNOT ready\n");
    // SYSTEM_SCHEDULER->yield();
    SYSTEM_SCHEDULER->yieldDisk();
  }

    Console::puts("\nready it IS\n");

  /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }

  SimpleDisk::read(_block_no, _buf);

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

  issue_operation(DISK_OPERATION::WRITE, _block_no);

  while(!is_ready()) {
    Console::puts("\nNOT wridy\n");
    bool disk = true;
    // SYSTEM_SCHEDULER->yield();
    SYSTEM_SCHEDULER->yieldDisk();
  }

    Console::puts("\nwridy it IS\n");

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }

}
