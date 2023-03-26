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

    pool_list_size = 0;

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
    page_table[1023] = (unsigned long) page_table | 0b011;
    page_directory[1023] = (unsigned long) page_directory | 0b011;
	
	
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
    // Console::puts("handling fault now...");
    // get faulting address
    unsigned long address32 = read_cr2();

    // check if address is legitimate
    bool legit = false;
    for(unsigned long i = 0; i < current_page_table->pool_list_size; i++) {
        if(current_page_table->pool_list[i]->is_legitimate(address32)) {
            legit = true;
            break;
        }
    }

    if(!legit) {
        Console::puts("[ERROR] Address is NOT legitimate");
        return;
    }
    

    // parse address 32
    unsigned long mask =  0b1111111111 << 12;
    unsigned long pgDirIndex = address32 >> 22; // first 10 bits
    unsigned pgTableIndex = (address32 & mask) >> 12; // middle 10 bits

    // get page directory entry
    unsigned long* directoryEntryAddress = current_page_table->PDE_address(pgDirIndex);
    unsigned long directoryEntry = *directoryEntryAddress;

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

        Console::puts("Handled page fault in page DIRECTORY\n\n\n");


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
    pool_list[pool_list_size++] = _vm_pool;

    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {

    //----- check if is page valid ------
    // parse address 32
    unsigned long address32 = _page_no;

    unsigned long mask =  0b1111111111 << 12;
    unsigned long pgDirIndex = address32 >> 22; // first 10 bits
    unsigned pgTableIndex = (address32 & mask) >> 12; // middle 10 bits

    // get page directory entry
    unsigned long* directoryEntryAddress = current_page_table->PDE_address(pgDirIndex);
    unsigned long directoryEntry = *directoryEntryAddress;

    bool pde_present = directoryEntry & 0b1;

    if(!pde_present) {
        Console::puts("PDE is NOT valid in free page");
        return;
    }

    // check if page table entry VALID
    unsigned long* pgTableEntryAddr = current_page_table->PTE_address(pgDirIndex, pgTableIndex);
    unsigned long pgTableEntry = *pgTableEntryAddr;

    bool pte_present = pgTableEntry & 0b1;

    if(!pte_present) {
        Console::puts("PTE is NOT valid in free page");
        return;
    }

    // --- free page ---
    unsigned long frameNumber = pgTableEntry >> 12;
    process_mem_pool->release_frames(frameNumber);


    // flush TLB
    write_cr3( (unsigned long) page_directory);
    
    Console::puts("freed page\n");
}
