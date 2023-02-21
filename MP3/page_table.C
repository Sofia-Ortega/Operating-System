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
	startAddress = kernel_mem_pool->get_frames(1) * PAGE_SIZE;
	unsigned long* page_table = (unsigned long*) startAddress; // start of pg table

	// first 4MB directly map
	unsigned long address = 0;
	for (unsigned int i = 0; i < 1024; i++) { // first 4MB
		page_table[i] = address | 0b011; // supervisor, read & write, present
		address = address + 4096;
	}

	page_directory[0] = *page_table; // directly mapped
	page_directory[0] |= 0b011; 

	// populate page directory
	// Note: already filled index 0 -> start at i = 1
	for (unsigned int i = 1; i < 1024; i++) { // first 4MB
		page_directory[i] = 0 | 0b010; // supervisor, read & write, NOT present
	}
	
	
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{

	current_page_table = this;
	write_cr3(*page_directory);

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
  assert(false);
  Console::puts("handled page fault\n");
}

