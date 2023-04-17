/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/
Scheduler* Scheduler::curr_scheduler = nullptr;

Scheduler::Scheduler() {

   /*
   initialize: 
      - ready queue
      - end of of quantum handler (eventually w RR)
      - idle thread
      - pointer to curr running thread
   
   */


  // idle thread
  char* stack_idle = new char[1024];
  idle_thread = new Thread(idle_function, stack_idle, 1024);
  curr_thread = idle_thread;


  // ready queue
  head = idle_thread;
  tail = idle_thread;

  // curr scheduler
  curr_scheduler = this; // FIXME: if running multiple schedulers

  Console::puts("Constructed Scheduler.\n");
}


void Scheduler::addToReadyQueue(Thread* thread) {
  Thread* new_head = head->next;
  while(new_head->terminated) {
    new_head = new_head->next;
  }

  // add old head to the back of the queue
  // ?????

}

void Scheduler::yield() {
  
  // temp var for old curr thread
  Thread* old_thread = curr_thread;

  // find new head
  Thread* new_head = head->next;
  while(new_head->terminated) {
    new_head = new_head->next;
  }

/*
  // clean up disk queue properly
  while(diskHead != nullptr && diskHead->terminated && diskHead != diskTail->next) {
    diskHead = diskHead->next;
  }
  if(diskHead == diskTail->next) {
    // disk queue is empty
    diskTail = nullptr;
    diskHead = nullptr;
  }*/

  // add old head to the back of the queue
  if(!old_thread->terminated) {
    tail->next = head;
    tail = tail->next;
  }

  tail->next = nullptr;
  
  // start queue in correct place
  head = new_head;

  // update curr thread 
  curr_thread = head;


  // context switch
  old_thread->dispatch_to(head);


  Console::puts("Yielded\n");

}

void Scheduler::yieldDisk() {
  // have thread cut in line with rest of disk queues

  curr_thread->disk = true;
  Thread* temp_thread = curr_thread;

  // will never be terminated if just yielding in disk


  // add to disk queue
  if(diskHead == nullptr) {
    diskHead = curr_thread;
    diskTail = curr_thread;
    curr_thread->next = nullptr;


    // add diskHead to ready queue
    tail->next = curr_thread;
    tail = diskTail;

  } else {
    // insert into ready queue
    curr_thread->next = diskTail->next;
    diskTail->next = curr_thread;
    diskTail = curr_thread;
  }


  yield();








}

void Scheduler::resume(Thread * _thread) {
  tail->next = _thread;
  tail = tail->next;
  
}

void Scheduler::add(Thread * _thread) {
  tail->next = _thread;
  tail = tail->next;

  Console::puts("Added thread");
}

void Scheduler::terminate(Thread * _thread) {
  Console::puts("Terminating Thread "); Console::puti(_thread->ThreadId()); Console::puts("\n");

  if(_thread == head) {
    head = head->next;
  }

  Thread* prev = head;
  Thread* curr = head->next;

  while(curr != nullptr && curr != _thread) {
    curr = curr->next;
    prev = prev->next;
  }


  if (curr == nullptr) {
    Console::puts("Terminating Thread not found in the queue");
    assert(false);
  }

  // repoint 
  prev = curr->next;


}
