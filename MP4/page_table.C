#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

unsigned long* PageTable::PDE_address(unsigned long index) {

    // 1023 -> 1023 -> index
    // 10 | 10 | 12
    unsigned long address32 = (1023 << 22) | (1023 << 12) | (index << 2);

    return (unsigned long*) address32;
}

unsigned long* PageTable::PTE_address(unsigned long pgDirIndex, unsigned long index) {
    // 1023 -> pgDirIndex -> index
    // 10 | 10 | 12

    unsigned long address32 = (1023 << 22) | (pgDirIndex << 12) | (index << 2);

    return (unsigned long*) address32;

}

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;

    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

	// frames * bytes / frames = bytes (address)
	unsigned long startAddress = kernel_mem_pool->get_frames(1) * PAGE_SIZE; 
	page_directory = (unsigned long*) startAddress;

	// page table -> array of pg table entries
	startAddress = process_mem_pool->get_frames(1) * PAGE_SIZE;
	unsigned long* page_table = (unsigned long*) startAddress; // start of pg table

	// first 4MB directly map
	unsigned long address = 0;
	for (unsigned int i = 0; i < 1024; i++) { // first 4MB
		page_table[i] = address | 0b011; // supervisor, read & write, present
		address = address + 4096;
	}


	page_directory[0] = (unsigned long) page_table; // directly mapped
	page_directory[0] |= 0b011; 

	// populate page directory
	// Note: already filled index 0 -> start at i = 1
	for (unsigned int i = 1; i < 1024; i++) { // first 4MB
		page_directory[i] = 0 | 0b010; // supervisor, read & write, NOT present
	}

    // recursive lookup
    page_table[1023] = (unsigned long) page_table;
    page_directory[1023] = (unsigned long) page_directory;
	
	
   Console::puts("Constructed Page Table object\n");
    // // frames * (bytes / frames) = bytes (address)
    // unsigned long startAddress = process_mem_pool->get_frames(1) * PAGE_SIZE;
    // page_directory = (unsigned long*) startAddress;


    // // page table
    // unsigned long startAddress2 = process_mem_pool->get_frames(1) * PAGE_SIZE;
    // unsigned long* page_table = ( unsigned long* ) ( startAddress2 );

    // // first 4MB directly mapped
    // unsigned long address = 0;
    // for(unsigned int i = 0; i < 1024; i++) {
    //     page_table[i] = address | 0b011;
    //     address += 4096;
    // }

    // // recursive
    // page_table[1023] = (unsigned long) page_table;

    // // connect page directory and page table
    // page_directory[0] = (unsigned long) page_table;
    // page_directory[0] |= 0b011;


    // // populate page directory
    // for(unsigned int i = 0; i < 1023; i++) {
    //     page_directory[i] = 0 | 0b010; // supervisor, read & write, NOT present
    // }

    // // recursive page look up
    // page_directory[1023] = (startAddress) | 0b011; // supervisor, read & write, present


    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long) page_directory);

    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    // set last bit in cr0 register
    unsigned long cr0Bits = read_cr0();

    unsigned long mask = 0b1 << 31;
    cr0Bits |= mask;
    write_cr0(cr0Bits);

    // update static variable
    paging_enabled = 1;


    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{

    // get faulting address
    unsigned long address32 = read_cr2();

    // parse address 32
    unsigned long mask =  0b1111111111 << 12;
    unsigned long pgDirIndex = address32 >> 22; // first 10 bits
    unsigned pgTableIndex = (address32 & mask) >> 12; // middle 10 bits


    // get page directory entry
    unsigned long* directoryEntryAddress = current_page_table->PDE_address(pgDirIndex);
    unsigned long directoryEntry = *directoryEntryAddress;

    // check if pg directory entry VALID
    bool present = directoryEntry & 0b1;

    if(!present) {
        // assign frame for page directory entry
        unsigned long frameNumber = process_mem_pool->get_frames(1);
        *directoryEntryAddress = (frameNumber * PAGE_SIZE) | 0b11; // set present & read/write

        // initialize page table entries to 0
        for(unsigned int i = 0; i < 1023; i++) {
            unsigned long* pageTableAddress = current_page_table->PTE_address(pgDirIndex, i);
            *pageTableAddress = 0;
        }

        // recursive page table lookup 
        unsigned long* pageTableAddress = current_page_table->PTE_address(pgDirIndex, 1023);
        *pageTableAddress = (unsigned long) current_page_table->PTE_address(pgDirIndex, 0);

        return;
    }

    
    // check if page table entry VALID
    unsigned long* pgTableEntryAddr = current_page_table->PTE_address(pgDirIndex, pgTableIndex);
    unsigned long pgTableEntry = *pgTableEntryAddr;

    present = pgTableEntry & 0b1;

    if(!present) {
        unsigned long frameNumber = process_mem_pool->get_frames(1);
        
        *pgTableEntryAddr = (frameNumber * PAGE_SIZE) | 0b11; // present

        Console::puts("Handled page fault in page TABLE\n");
        return;
    }
    
    Console::puts("[WARNING] Page Fault NOT handled. Page directory & table marked present \n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{

/*
    if ( poolHead == nullptr ) {
        poolHead = &poolNode(_vm_pool, nullptr);
    } else {
        poolHead = &poolNode(_vm_pool, poolHead);
    }
*/

    Console::puts("NOT registered VM pool\n");
    assert(false);
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page\n");
}
