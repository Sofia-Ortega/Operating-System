/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
	unsigned bitmap_index = _frame_no / 4;
	int pickBit = _frame_no % 4;

	// if pickBit == 0;   00 00 00 11 
	// elif pickBit == 1; 00 00 11 00 
	// elif pickBit == 2; 00 11 00 00
	// elif pickBit == 3; 11 00 00 00 
	
	pickBit *= 2;
	unsigned char mask = 0x3 << pickBit;

	

	unsigned char c = bitmap[bitmap_index];

	// Free: 00
	// Used: 01
	// HoS:  11 
	
	unsigned char bitPair = c & mask; // 11 00 00 00
	bitPair = bitPair >> pickBit; // 00 00 00 11   

	if (bitPair == 0x00) {
		return FrameState::Free;
	} else if (bitPair == 0x1) {
		return FrameState::Used;
	} else if(bitPair == 0x3) {
		return FrameState::HoS;
	} else if (bitPair == 0x2) {
		Console::puts("get_state: bitPair result: 2 WRONG \n ");
		assert(false);
	} else {
		Console::puts ("get_state: something went TERRIBLY wrong \n");
		assert(false);
	}

	/*
	switch(c) {
		case (c & mask) == 0:
			return FrameState::Free;
			break;
			
		case intBitPair == 1:
			return FrameState::Used;
			break;
		case intBitPair == 3:
			return FrameState::HoS;
			break;
		
	}
	*/


}


// TODO: eventually, use BIT MANIPULATION
void ContFramePool::set_state(unsigned long _frame_no, FrameState _state) {
	unsigned bitmap_index = _frame_no / 4;
	int pickBit = _frame_no % 4;

	// if pickBit == 0;   00 00 00 11 
	// elif pickBit == 1; 00 00 11 00 
	// elif pickBit == 2; 00 11 00 00
	// elif pickBit == 3; 11 00 00 00 
	
	pickBit *= 2;
	unsigned char mask = 0x3 << pickBit;
	

	// Free: 00
	// Used: 01
	// HoS:  11 

	// (unsigned char) _state
	// 00 out then or
	switch(_state) {
		case FrameState::Free:
			// 11 & 00 = 00
			bitmap[bitmap_index] &= ~mask;
			break;
		case FrameState::Used:
			// convert bit pair to 00
			bitmap[bitmap_index] &= ~mask;
			
			mask = 0x01 << pickBit;
			bitmap[bitmap_index] |= mask;
			break;
		case FrameState::HoS:
			
			bitmap[bitmap_index] |= mask;
			break;
	}
	
	Console::puts("\nasserting get_state: ");
	Console::puti((int)_state);
	assert(get_state(_frame_no) == _state);
	Console::puts("done get_state \n");
}



ContFramePool* ContFramePool::head = nullptr;

 // Constructor: Initialize all frames to FREE, except for any frames that you 
 // need for the management of the frame pool, if any.
ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
	Console::puts("Frame Pool (theoretically) initialized\n");

	// adding to linked list
	if(head == nullptr) {
		head = this;
		next = nullptr;
		prev = nullptr;
	} else {
		next = head;
		prev = nullptr;
		head->prev = this;
		head = this;
	}
	
	// HOW do i know how much i need to manage frames??

	// Bitmpa must fit into single frame
	// assert(_n_frames <= FRAME_SIZE * 4);

	base_frame_no = _base_frame_no;
	nframes = _n_frames;
	info_frame_no = _info_frame_no;

	// if _info_frame_no is zero, then we keep management info in the first
	// frame, else we use the provided frame to keep managment info
	if(info_frame_no == 0) {
		bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
	} else {
		bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
	}
	
	
	
	unsigned long neededFrames = needed_info_frames(_n_frames);
	base_frame_no += neededFrames; 
	nFreeFrames -= neededFrames;
	

	// Everything fine-and-dandy. Proceed to mark all frames as free
	
	for(int fno = 0; fno < nframes; fno++) {
		// Console::puts ( "set_state here");
		set_state(fno, FrameState::Free); 
	}
	


	// mark first frame as used, if it is being used 
	
/*	
	if(info_frame_no == 0) {
		set_state(0, FrameState::HoS);	
		nFreeFrames--;
	}
*/	
	



}



 // get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 // sequence of at least _n_frames entries that are FREE. If you find one, 
 // mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 // ALLOCATED.
unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
   
	Console::puts("get_frames called: \n ");

	// any frames left to allocate?
	assert(nFreeFrames >= _n_frames);


	
	// find first frame_no of sequence of free frames
	// mark frame seq as used in bitmap 
	unsigned int  frame_no = 0;
	unsigned int counter = 0;

	while(counter < _n_frames) {
		if(get_state(frame_no) == FrameState::Free) {
			counter++;
		} else {
			counter = 0;
		}

		frame_no++;
	}

	
	frame_no -= _n_frames; // get first frame_no of sequence

	mark_inaccessible(frame_no, _n_frames);
	/*
	// mark first frame in seq as HoS
	set_state(frame_no, FrameState::HoS);

	// mark all frames in range as used
	for(unsigned int i = frame_no + 1; i < frame_no + _n_frames - 1; i++) {
		set_state(i, FrameState::Used);
	}	

	nFreeFrames -= _n_frames;

	// don't have to check if overrun. Handled by assert above
	Console::puts("ContframePool::get_frames working ?? :D\n");
	
	*/
    	return frame_no + base_frame_no;
}


/*
   mark_inaccessible(_base_frame_no, _n_frames): This is no different than
   get_frames, without having to search for the free sequence. You tell the
   allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
   frames after that to mark as ALLOCATED.
*/
void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
	
	// mark first frame in seq as HoS
	set_state(_base_frame_no, FrameState::HoS);

	for (int fno = _base_frame_no + 1; fno < _base_frame_no + _n_frames; fno++) {
		set_state(fno, FrameState::Used); // what is this minus buisness??
	}

	nFreeFrames -= _n_frames;
	
    Console::puts("ContframePool::mark_inaccessible MIGHT work\n");
}

/*
 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.

 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
*/

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
	

	// find correct frame pool
	ContFramePool* pool = head;
	unsigned long frameNum = 0;

	while (pool != nullptr) {
		frameNum = _first_frame_no + pool->base_frame_no;
		if (_first_frame_no >= pool->base_frame_no && _first_frame_no < pool->base_frame_no + pool->nframes) break;
		// if( pool->get_state( frameNum ) == FrameState::HoS) break;	
		pool = pool->next;
	}


	// error checking: pool not found
	if (pool == nullptr) {
		Console::puts("ContframePool::Release_frames: pool not found");
		return;
		// assert(false);
	}

	// release frames from pool
	frameNum = _first_frame_no;
	while (pool->get_state(frameNum) != FrameState::Free) {
		pool->set_state(frameNum, FrameState::Free); 
		frameNum++;
	}

}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	int round ;
	// convert nframes to bytes
	// Note: 4 frames / bytes
	// frames * ( 1 byte / 4 frames) = bytes
	unsigned long neededBytes = _n_frames / 4;  
	neededBytes += (_n_frames % 4) == 0 ? 0 : 1; // round


	// 1 frame / FRAME_SIZE bytes 
	// neededBytes * (1 frame / FRAME_SIZE bytes) = frames 
	unsigned long neededFrames = neededBytes / FRAME_SIZE;
	neededFrames += (neededBytes % FRAME_SIZE) == 0 ? 0 : 1;

	return neededFrames;
}
