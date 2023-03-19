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


    // use logical address 1 | 0 | 0 for list
    free_list = 0b1 << 22;

    // use logical address 1 | 1 | 0 for allocated list
    allocated_list = (0b1 << 22) | (0b1 << 12);
    allocated_list_size = 0;


    // even indexes: base addresses
    // odd indexes: no of pages allocated 
    free_list[0] = _base_address;
    free_list[1] = _size / _page_table->PAGE_SIZE;
    free_list_size = 1;


    

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    // number of pages needed
    unsigned long needed_pages = _size / page_table->PAGE_SIZE;
    if (_size % page_table->PAGE_SIZE != 0) needed_pages++; // rounding

    for(unsigned long i = 0; i < free_list_size * 2; i += 2) {
        unsigned long available_pages = free_list[i + 1];
        unsigned long available_address = free_list[i];

        if(available_pages > needed_pages) {
            // split free list

            // add to end of allocated list
            allocated_index = allocated_list_size * 2;
            allocated_list[allocated_index] = available_address;
            allocated_list_size += 2;

            // update free list
            free_list[i] = available_address + page_table->PAGE_SIZE;// shift down one page
            free_list[i+1] = available_pages - needed_pages;

            Console::puts("Allocated region of memory.\n");
            return available_address;
        }
    }



    Console::puts("NOT Allocated region of memory.\n");
    return 0;
}

void VMPool::release(unsigned long _start_address) {
    assert(false);
    


    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {

    Console::puts("Checking whether address is part of an allocated region.\n");

    if(_address == (unsigned long) free_list || _address == (unsigned long) allocated_list) {
        return true;
    }

    // FIXME
    if (_address > base_address && _address < base_address + size) {
        return true;
    }


    return false;
}

