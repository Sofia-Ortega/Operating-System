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
  curr_scheduler = this; 

  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  
  Machine::disable_interrupts();

  // temp var for old curr thread
  Thread* old_thread = curr_thread;

  // find new head
  Thread* new_head = head->next;
  while(new_head->terminated) {
    new_head = new_head->next;
  }

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

  // curr_thread->startTimer();

  // context switch
  old_thread->dispatch_to(head);


  Console::puts("Yielded\n");

  Machine::enable_interrupts();

}

void Scheduler::resume(Thread * _thread) {
  Machine::disable_interrupts();

  tail->next = _thread;
  tail = tail->next;

  Machine::enable_interrupts();
  
}

void Scheduler::add(Thread * _thread) {
  Machine::disable_interrupts();

  tail->next = _thread;
  tail = tail->next;

  Console::puts("Added thread");

  Machine::enable_interrupts();
}

void Scheduler::terminate(Thread * _thread) {

  // once start terminating, want to finish
  Machine::disable_interrupts();

  Console::puts("Terminating Thread ");
  Console::puti(_thread->ThreadId()); 
  Console::puts("\n");

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

  // once finish terminating, want to finish
  Machine::enable_interrupts();


}
