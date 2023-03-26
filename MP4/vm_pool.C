/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {

    
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;


    // register pool
    _page_table->register_pool(this);

    // use logical address 1 | 0 | 0 for list
    free_list = (unsigned long *) base_address;

    // use logical address 1 | 1 | 0 for allocated list
    allocated_list = (unsigned long* ) (base_address + page_table->PAGE_SIZE);
    allocated_list_size = 0;


    // even indexes: base addresses
    // odd indexes: number of pages  
    free_list[0] = _base_address + 2*_page_table->PAGE_SIZE; // start 2 frames after where lists stored
    free_list[1] = _size / _page_table->PAGE_SIZE;
    free_list_size = 1;
    

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    // number of pages needed
    unsigned long needed_pages = _size / page_table->PAGE_SIZE;
    if (_size % page_table->PAGE_SIZE != 0) needed_pages++; // rounding


    // find free space in list
    for(unsigned long i = 0; i < free_list_size * 2; i += 2) {
        unsigned long available_address = free_list[i];
        unsigned long available_pages = free_list[i + 1];

        if(available_pages > needed_pages) {
            // split free list

            // add to end of allocated list
            unsigned long allocated_index = allocated_list_size * 2;
            allocated_list[allocated_index] = available_address;
            allocated_list[allocated_index + 1] = needed_pages;
            allocated_list_size++;

            // update free list
            free_list[i] = available_address + (needed_pages * page_table->PAGE_SIZE);// shift down needed page
            free_list[i+1] = available_pages - needed_pages;

            Console::puts("Allocated region of memory: \n");
            return available_address;
        }
    }



    Console::puts("NOT Allocated region of memory.\n");
    return 0;
}

void VMPool::release(unsigned long _start_address) {
    
    for(unsigned long i = 0; i < allocated_list_size * 2; i += 2) {
        if(_start_address == allocated_list[i]) {

            // add back to free list
            unsigned long free_list_index = free_list_size * 2;
            free_list[free_list_index] = allocated_list[i];
            free_list[free_list_index + 1] = allocated_list[i+1];

            // release each page allocated
            unsigned long num_pages = allocated_list[i+1];
            unsigned long address = _start_address;
            for(unsigned long j = 0; j < num_pages; j++) {
                page_table->free_page(address);
                address += page_table->PAGE_SIZE;
            }

            // // remove from allocated list
            unsigned long last_index = (allocated_list_size - 1) * 2;
            allocated_list[i] = allocated_list[last_index];
            allocated_list[i+1] = allocated_list[last_index + 1];

            // update sizes of list
            allocated_list_size--;
            free_list_size++;


            Console::puts("Released region of memory:\n");
            Console::putui(_start_address);
            Console::puts("\n");
            Console::putui(allocated_list[i]);
            Console::puts("\n");
            Console::putui(allocated_list_size);
            Console::puts("\n");

            return;
        }
    }

    Console::puts("[WARNING] Failed to release region of memory.\n");

}

bool VMPool::is_legitimate(unsigned long _address) {

    Console::puts("Checking whether address is part of an allocated region.\n");

    Console::puts("_address: ");
    Console::putui( _address);

    if (_address < base_address || _address > base_address + size) return false;

    // mark first 2 frames holding free list and allocated list as legit
    if(_address >= (unsigned long ) free_list 
        && _address < ((unsigned long) allocated_list ) + page_table->PAGE_SIZE) return true;

    // look for address in allocated list
    for(unsigned long i = 0; i < allocated_list_size * 2; i += 2) {

        unsigned long i_address = allocated_list[i];
        Console::puts("address");
        Console::putui(i_address);
        unsigned long i_size = allocated_list[i+1] * page_table->PAGE_SIZE;

        if (_address >= i_address 
            && _address <= i_address + i_size) return true;
    }



    return false;
}

