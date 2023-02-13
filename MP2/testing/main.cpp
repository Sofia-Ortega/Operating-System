#include <iostream>
#include "simple_frame_pool.H"
using namespace std;

int main() {

	 SimpleFramePool pool(16, 4, 0);
	// Check Linked List
/*
	for(unsigned long i = 0; i < 100; i*=10) {
		ContFramePool pool(i, (unsigned long)5, (unsigned long) 0);
	}

*/

	// ContFramePool* h = ContFramePool::head;
	

/*
	cout << pool.base_frame_no << endl;
	cout << pool.nframes << endl;
	cout << pool.info_frame_no << endl;

*/
}
