/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>


// PeakMemoryUtilization =
// (historically_the_highest_amount_of_payload_existed_at_once_in_the_heap) / maximum_size_of_the_heap


/**
 * You should store the head of your free list in this variable.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
sf_free_header* freelist_head = NULL;
int remaining_page_byte = 0;
bool first_malloc = true;
void *original_adr;
void *end_of_page_adr;
void *prev_adr;
void *current_adr;
void *footer_ptr;
void *page_footer;
int page_number = 0;
sf_free_header header_one;
sf_free_header header_two;
sf_footer footer_one;
sf_header header;
sf_free_header *freelist_head;
bool is_splinter = false;
sf_header free_header;
sf_header o;
double payload_size_in_heap;
double heap_size;


static int allocatedBlocks;
static int splinterBlocks;
static int padding;
static int splintering;
static int coalesces;
static double peakMemoryUtilization;


bool check_free_list(void* ptr){
	sf_free_header* temp = freelist_head;
	bool in_list = false;
	while(temp != NULL){
		if(temp == ptr){
			in_list = true;
			return in_list;
			break;
		}
		temp = temp->next;
	}
	return in_list;
}

void create_freelist_header(void* h,int size){

  int number_of_pages = ((size/4096) + 1) * 4096;
  page_number += (size/4096) + 1;

  freelist_head = (sf_free_header*)h;
  freelist_head->next = NULL;
  freelist_head->prev = NULL;
  freelist_head->header.block_size = number_of_pages>>4;
  freelist_head->header.alloc = 0;

  page_footer = (char*)freelist_head + freelist_head->header.block_size*16 - SF_FOOTER_SIZE;
  sf_footer* footer = (void*)page_footer;
  footer->block_size = number_of_pages>>4;
  footer->alloc = 0;
}

void setfooter(void* footer_ptr){
		((sf_footer*)footer_ptr)->alloc = 1;
		((sf_footer*)footer_ptr)->block_size = header.block_size;
		((sf_footer*)footer_ptr)->splinter = 0;
}

void sort_freelist(void* ptr){
	void* array_size[500];
	int y = sizeof(array_size);
	y+=0;
	int len = 1;
	array_size[0] = ptr;
	sf_free_header* iterator = freelist_head;

	while(iterator != NULL){
		array_size[len] = (void*)iterator;
		iterator = iterator->next;
		len++;
	}

	void* holder = NULL;
	for(int i = 0 ; i < len; ++i){
		for(int j = i + 1; j < len;++j){
			if(array_size[i] > array_size[j]){
				holder = array_size[i];
				array_size[i] = array_size[j];
				array_size[j] = holder;
			}
		}
	}

	freelist_head = NULL;
	freelist_head = array_size[0];
	freelist_head->next = NULL;
	freelist_head->prev = NULL;
	sf_free_header* temp = freelist_head;

	for(int i = 1 ; i < len ; i++){
		void* freelist_temp = temp;

		temp->next = array_size[i];
		temp = temp->next;
		temp->prev = freelist_temp;
	}
}

bool call_sbrk(int size){
	void* adr = sf_sbrk(0);
	adr = (char*)adr + 0;
	int x = size;
	void* new_sbrk;
	while(x> 0){
		new_sbrk = sf_sbrk(1);
		if(page_number>4){
			return false;
		}
		page_number++;
		x -= 4096;
	}
	if(page_number > 4){
		return false;
	}
	//void* new_sbrk = sf_sbrk((size/4096)+1);
	new_sbrk = (char*)new_sbrk + 0;
	void* new_header = (char*)freelist_head;
	sf_header* h = new_header;
	h->block_size = h->block_size + (((size/4096+1)*4096)/16);

	void* new_footer = (char*)h + h->block_size*16 - SF_HEADER_SIZE ;
	sf_footer* f = new_footer;
	end_of_page_adr = f;
	f->block_size = h->block_size;
	return true;
}

void* find_bestfit(int block_size, int size){
	sf_free_header* temp = freelist_head;
	int block_size_arr[500];
	void* adr_arr[500];

	for(int i = 0 ; i < 499 ; i++){
		adr_arr[i] =  0;
		block_size_arr[i] = 0;
 	}

	int c = 0;
	is_splinter = false;
	bool not_enough_room = true;
	int largest_block = 0;
   	while(temp != NULL){
   		if(temp->header.block_size == (block_size/16) + 1){
   			not_enough_room = false;
   			return (void*)temp;
   		}
   		else if(temp->header.block_size >= block_size/16 + 1){
   			not_enough_room = false;
   			block_size_arr[c] = temp->header.block_size - (block_size/16);
   			adr_arr[c] = (void*)temp;
   			c++;
   		}

   		if(largest_block < temp->header.block_size*16){
   			largest_block = temp->header.block_size*16;
   		}

  		temp = temp->next;
   	}

   	if(not_enough_room == true){
   		bool x = call_sbrk(size - largest_block);
   		if(x == false){
   			return NULL;
   		}

   	}

   	if(freelist_head->next == NULL){
   		if(freelist_head->header.block_size - (block_size/16) - 1 == 1){
   			is_splinter = true;
   			freelist_head->header.splinter = 1;
   			freelist_head->header.splinter_size = 16;

   		}
		return (char*)freelist_head ;
	}



   	int z = 0;
   	while(block_size_arr[z] == 0){
   		z++;
   	}
   	int minimum = block_size_arr[z];
   	int location = z;
   	int counter = 0;
   	for(int i = z ; i < c  ; i++){
   		if(block_size_arr[i] < minimum && block_size_arr[i] != 0){
   			minimum = block_size_arr[i];
   			location = z ;//+ 1;
   			counter++;
   		}
   	}

   	while(temp != NULL){
   		if(adr_arr[location] == (void*)temp){
   			break;
   		}
   		temp = temp->next;
   	}
   	temp = freelist_head;

   	if(temp->header.block_size*16 - ((block_size)) == 32){
   		is_splinter = true;
   		temp->header.splinter_size = 16;
   		temp->header.splinter = 1;
   	}
   	else{
   		is_splinter = false;
   	}
	return adr_arr[location];
}
int calc_padding(int size){
	while(size%16 != 0){
		size++;
	}
	return size;
}

void* set_block(void* adr,int size, int padded_size){
	if(freelist_head == NULL){
		freelist_head = adr;
	}
	sf_free_header* sf = (sf_free_header*)adr;

	int orig_bs = sf->header.block_size;
	int inc = 0;

	if(is_splinter == false){
		header = setheader(header,size);
		header.splinter = 0;
		header.splinter_size = 0;
		sf->header = header;
		inc = header.block_size ;

		void* allocated_footer_ptr = (char*)sf + sf->header.block_size*16 - SF_FOOTER_SIZE;
		sf_footer* allocated_footer = allocated_footer_ptr;

		allocated_footer->block_size = sf->header.block_size;
		allocated_footer->alloc = sf->header.alloc;
		allocated_footer->splinter = sf->header.splinter;
		padding += header.padding_size;
	}
	else{

		sf->header.alloc = 1;
		int temp = 0;
		while(size%16!=0){
			temp++;
			size++;
		}
		sf->header.padding_size = temp;
		int req_size = padded_size - temp;
		sf->header.requested_size = req_size;
		inc = sf->header.block_size - (size/16);

		void* v_f = (char*)sf + sf->header.block_size*16 - SF_FOOTER_SIZE;
		sf_footer* f = v_f;
		f->block_size = sf->header.block_size;
		f->splinter = 1;
		f->alloc = 1;

		padding += sf->header.padding_size;

	}

	void* update = (char*)sf + inc*16 ;
	void* next = sf->next;
	void* prev = sf->prev;

	if(orig_bs - (size/16) == 0){
		while(freelist_head != (void*)sf){
			freelist_head = freelist_head->next;
		}
		if(freelist_head->prev != NULL && freelist_head->next == NULL){
			freelist_head = freelist_head->prev;
			freelist_head->next = next;
		}
		if(freelist_head->next != NULL && freelist_head->prev == NULL){
			void* temp = (void*)freelist_head;
			freelist_head->next->prev = temp;
			freelist_head = freelist_head->next;
			freelist_head->prev = NULL;
		}
		while(freelist_head->prev != NULL){
			freelist_head = freelist_head->prev;
		}
		if(freelist_head->next == NULL && freelist_head->header.block_size == orig_bs){
			freelist_head = NULL;
		}

		return (char*)adr + SF_FOOTER_SIZE;
	}
	sf_header* update_head = (sf_header*)update;

	// checks if the header is already allocated if it is then delete the reference
	if(update_head->alloc != 0){
		bool in_list = check_free_list((void*)update_head);
		if(in_list == false){
			void* prev = freelist_head->prev;
			if(freelist_head->next != NULL){
				freelist_head = freelist_head->next;
				freelist_head->prev = prev;
			}
			else{
				freelist_head = NULL;
			}
		}
		return (char*)adr + SF_FOOTER_SIZE;
	}


	update_head->block_size = orig_bs - ((padded_size/16)+1);
	update_head->alloc = 0;
	void* u_f = (char*)update_head + update_head->block_size*16 - SF_FOOTER_SIZE;

	sf_footer* update_foot = (void*) u_f ;
		update_foot->block_size =  update_head->block_size;
		update_foot->alloc = 0;



	int counter = 0;
	while(freelist_head != (void*)sf){
		counter++;

		freelist_head = freelist_head->next;
	}

	freelist_head = (void*)update_head;
	freelist_head->next = next;
	freelist_head->prev = prev;

	if(freelist_head->next != NULL){
		freelist_head->next->prev = (void*)update_head;
	}
	if(freelist_head->prev != NULL){
		freelist_head->prev->next = (void*)update_head;
	}

	while(freelist_head->prev != NULL){
		freelist_head = freelist_head->prev;
	}
	if(freelist_head == end_of_page_adr){
		freelist_head = NULL;
	}
	if(update_head->block_size == 0){
		freelist_head = NULL;
	}

	void* ret = (char*)adr + SF_FOOTER_SIZE;
	return (char*)ret;
}

void* sf_malloc(size_t size) {
	if(remaining_page_byte + size + 16 > 16384)
	{
		int y = calc_padding(size);
		remaining_page_byte = remaining_page_byte + 16 + y;
		errno = ENOMEM;
		return NULL;
	}
	if(remaining_page_byte == 16384 || remaining_page_byte > 16384){
		errno = ENOMEM;
		return NULL;
	}
	if(size > 16368){
		errno = ENOMEM;
		return NULL;
	}
	if(page_number > 4){
		errno = ENOMEM;
		return NULL;
	}
	if(size == 0 ){
		errno = EINVAL;
		return NULL;
	}
	if(first_malloc == true){
		first_malloc = false;						
		original_adr = sf_sbrk(0);
		int x = size;
		while(x >= 0){
			if(page_number > 4){
				errno = ENOMEM;
				return NULL;
			}
			original_adr = sf_sbrk(1);
			x -= 4096;
		}
		create_freelist_header(original_adr,size);

		int eop = ((size/4096)+1)*4096;
		end_of_page_adr = (char*)original_adr + eop;

		current_adr =(char*)original_adr ;

		//setting up first header
		header = setheader(header,size);

		if(eop - header.block_size*16 == 16){
			header.splinter = 1;
			header.splinter_size = 16;
			header.block_size += 1;

			freelist_head = current_adr;
			freelist_head->header = header;

			void* free_footer_ptr = (char*)freelist_head + freelist_head->header.block_size*16  - SF_FOOTER_SIZE;
			sf_footer* free_footer = free_footer_ptr;
			free_footer->block_size = freelist_head->header.block_size;
			free_footer->alloc = 1;
			free_footer->splinter = 1;

			freelist_head = NULL;
			remaining_page_byte += header.block_size*16;
			allocatedBlocks+=1;

	payload_size_in_heap += (header.block_size*16) - 16;
	heap_size = (char*)end_of_page_adr - (char*)original_adr;
	peakMemoryUtilization = payload_size_in_heap/heap_size;

			splinterBlocks+=1;
			splintering += 16;
			padding += header.padding_size;
			return (char*)current_adr + SF_HEADER_SIZE ;
		}

		footer_ptr = (char*)current_adr + (header.block_size*16) - SF_FOOTER_SIZE ;
		setfooter(footer_ptr);

		void* flhp = (char*)footer_ptr + SF_FOOTER_SIZE;

		freelist_head = current_adr ;
		freelist_head->header = header;

		freelist_head = flhp;

		freelist_head->next = NULL;
		freelist_head->prev = NULL;

		freelist_head->header.block_size = (eop/16) - header.block_size;// temp.block_size - header.block_size;

		int footer_block_size = freelist_head->header.block_size;

		sf_footer* end_footer;
		void* f = (char*)end_of_page_adr - SF_FOOTER_SIZE;
		end_footer = f;
		end_footer->block_size = footer_block_size;//freelist_head->header.block_size;

		remaining_page_byte += header.block_size*16;

		if(header.block_size*16 == 16384){
			end_footer->block_size = 1024;
			freelist_head = NULL;
		}
		if(header.block_size*16 == 4096){
			end_footer->block_size = 256;
			freelist_head = NULL;
		}
		if(header.block_size*16 == 8192){
			end_footer->block_size = 512;
			freelist_head = NULL;
		}
		if(header.block_size*16 == 12288){
			end_footer->block_size = 768;
			freelist_head = NULL;
		}
		allocatedBlocks+=1;

	payload_size_in_heap += (header.block_size*16) - 16;
	heap_size = (char*)end_of_page_adr - (char*)original_adr;
	peakMemoryUtilization = payload_size_in_heap/heap_size;

		padding+=header.padding_size;
		return (char*)current_adr + SF_HEADER_SIZE ;
	}


	if(freelist_head == NULL){
		int x = size;
		int y = 0;
		freelist_head = sf_sbrk(0);
		while(x > 0){
			if(page_number > 4){
				errno = ENOMEM;
				return NULL;
			}
			page_number++;
			sf_sbrk(1);
			end_of_page_adr = (char*)end_of_page_adr + 4096;
			x -= 4096;
			y += 4096;
		}
		sf_header edgecase = freelist_head->header;
		edgecase = setheader(edgecase,size);

		if((((size/4096)+1)*4096)/16 - edgecase.block_size == 1){
			void* fh_p = (char*)freelist_head;
			sf_header* fh = fh_p;
			fh->alloc = 1;
			fh->block_size = edgecase.block_size + 1;
			fh->requested_size = size;
			fh->splinter = 1;
			fh->splinter_size = 16;

			void* ff_p = (char*)fh + fh->block_size*16 - SF_FOOTER_SIZE;
			sf_footer* fp = ff_p;
			fp->alloc = 1;
			fp->block_size = fh->block_size;
			fp->splinter = 1;

			freelist_head = NULL;
			remaining_page_byte += fh->block_size*16;
			allocatedBlocks+= 1;

	payload_size_in_heap += (fh->block_size*16)-16;
	heap_size = (char*)end_of_page_adr - (char*)original_adr;
	peakMemoryUtilization = payload_size_in_heap/heap_size;

			return (char*)fh_p + 8;
		}


		void* f_h_ptr = (char*)freelist_head;
		sf_header* f_h = f_h_ptr;
		f_h->block_size = (((size/4096)+1)*4096)/16;
		f_h->alloc = 0;

		void* f_f_ptr = (char*)f_h + f_h->block_size*16 - SF_FOOTER_SIZE;
		sf_footer* f_f = f_f_ptr;
		f_f->block_size = f_h->block_size;
		f_f->alloc = 0;
	}

	int padded_size = calc_padding(size);
	int x = padded_size;
	void* bestfit = find_bestfit(padded_size,size);
	if(is_splinter == true){
		size += 32;
		splinterBlocks+=1;
		splintering+=16;

		x = calc_padding(size - 32) + 16;
	}
	remaining_page_byte += padded_size + 16;

	payload_size_in_heap += x;
	heap_size = (char*)end_of_page_adr - (char*)original_adr;
	peakMemoryUtilization = payload_size_in_heap/heap_size;

	allocatedBlocks += 1;

	return (char*)set_block(bestfit,size,padded_size);
}

void *sf_realloc(void *ptr, size_t size) {
	// first check if ptr is valid
	// then check if size is zero then just call sf_free
	// check if we can still call sf_sbrk

	/* when ptr is invalid then set errno to enomem*/

	if(ptr == NULL){
		errno = EINVAL;
		return NULL;
	}

	bool valid_bound = check_valid_bounds(ptr);
	bool valid_ptr = check_valid_free(ptr);


	if((int)size < 0){
		errno = EINVAL;
		return NULL;
	}

	if(valid_ptr == false || valid_bound == false){
		errno = EINVAL;
		return NULL;
	}
	if(size == 0){
		allocatedBlocks -=1;
		sf_free(ptr);
		return NULL;
	}

	int orig_size = size;
	size = calc_padding(size);
	void* head_ptr = (char*)ptr - 8;
	sf_header* header = head_ptr;

	int orig_bs = header->block_size;
	orig_bs += 0;
	int orig_padding = header->padding_size;
	int payload_size = header->splinter_size + header->padding_size + header->requested_size;

	if(payload_size > size){
		//shrinking

		// erase the footer
		void* erase_foot_ptr = (char*)header + header->block_size*16 - SF_FOOTER_SIZE;
		sf_footer* erase_foot = erase_foot_ptr;
		erase_foot->block_size = 0;
		erase_foot->alloc = 0;
		erase_foot->splinter = 0;

		// update allocated header
		header->requested_size = orig_size;
		int padded_size = calc_padding(size);
		header->block_size = (padded_size/16) + 1 ;
		header->alloc = 1;

		int temp_pad = orig_size;
		int pad_value = 0;
		while(temp_pad % 16 != 0){
			pad_value+=1;
			temp_pad+=1;
		}
		header->padding_size = pad_value;
		padding += pad_value;
		padding -= orig_padding;

		if(orig_bs - header->block_size == 1){
			//splinter
			header->block_size += 1;
			header->splinter = 1;
			header->splinter_size = 16;
			splinterBlocks +=1;
			splintering+=16;
		}

		// create allocated new footer
		void* new_footer_ptr = (char*)header + header->block_size*16 - SF_FOOTER_SIZE;
		sf_footer* new_footer = new_footer_ptr;
		new_footer->block_size = header->block_size;
		new_footer->alloc = 1;
		new_footer->splinter = 0;

		void* new_link = (char*)new_footer + 8;

		sf_header* potential_header = new_link;
		if(potential_header->alloc == 0){
			//coalesces+=1;
		}


		sf_header* free_head = (sf_header*)new_link;
		int free_blk_size = (orig_bs - header->block_size);

		void* free_foot_ptr = (char*)new_link + free_blk_size*16 - SF_FOOTER_SIZE;
		sf_footer* free_foot = free_foot_ptr;



		if(free_blk_size != 0){
			// setting free head
			free_head->block_size = free_blk_size;
			free_head->alloc = 0;

			// setting free foot
			free_foot->block_size = free_blk_size;
			free_head->alloc = 0;
			sort_freelist(new_link);
			coalescing((char*)new_link+8);

		}

	double new_payload = header->requested_size + header->splinter_size	+ header->padding_size;

			payload_size_in_heap += (new_payload - payload_size);
			heap_size = (char*)end_of_page_adr - (char*)original_adr;
			peakMemoryUtilization = payload_size_in_heap/heap_size;

		return (char*)head_ptr + 8;

	}
	else if(payload_size < size){

		/*
    	Does it fit within the block right after? If so throw it in.
    	Does it not fit?
        Search for space in the list.
        Can allocate more heap space if you need to AFTER SEARCHING THE LIST.
        If there is no space, return error.
		*/

		// check if next block is a good fit and is large enough
		// dont forget to add to remaining page byte
		// check if we need to call another page

		void* next_p = (char*)head_ptr + header->block_size*16;
		sf_header* next_free_header = next_p;

		// if we need new page and if next block needs to create another page
		bool memory_issue = true;
		if( next_free_header->block_size*16 < size && next_free_header->alloc == 0){
			memory_issue = call_sbrk(size - (next_free_header->block_size*16) - orig_bs*16 +16);
		}
		if(memory_issue== false){
			errno = ENOMEM;
			return NULL;
		}

		if((header->block_size*16 + next_free_header->block_size*16) > size + 16
			&& next_free_header->alloc == 0){

			//0. check for splinter// next_block - new_size == 16??
			//1. set free header and footer
			//2. then calcualte the new header and footer
			//3. update freelist


			int new_size = calc_padding(orig_size) + 16;

			int temp_orig_size = orig_size;
			int pad = 0;
			while(temp_orig_size % 16 != 0 ){
				temp_orig_size++;
				pad++;
			}
			//padding += pad;

			/* set the splinter*/
			if(next_free_header->block_size*16 + orig_bs*16 - new_size == 16){
				new_size += 16;
				header->block_size = new_size/16;
				header->alloc = 1;
				header->requested_size = orig_size;
				header->splinter_size = 16;
				header->splinter = 1;
				header->padding_size = pad;
				splinterBlocks+=1;
				splintering+=16;

			}else{
			/* no splinter */
			header->requested_size = orig_size;
			header->alloc = 1;
			header->block_size = new_size/16;
			header->splinter = 0;
			header->splinter_size = 0;
			header->padding_size = pad;
			}
			/* calculate memory peak */

			double new_payload = header->requested_size + header->splinter_size
			+ header->padding_size;

			payload_size_in_heap += (new_payload - payload_size);
			heap_size = (char*)end_of_page_adr - (char*)original_adr;
			peakMemoryUtilization = payload_size_in_heap/heap_size;
			padding -= orig_padding;
			padding += header->padding_size;

			/* set up the linked list*/
			sf_free_header* temp = freelist_head;
			void* next = freelist_head->next;
			void* prev = freelist_head->prev;

			/* Set the allocated footer*/
			void* allocated_footer_ptr = (char*)header + header->block_size*16 - SF_FOOTER_SIZE;
			sf_footer* allocated_footer =  allocated_footer_ptr;

			int h_bs = header->block_size;
			allocated_footer->block_size = h_bs;
			allocated_footer->alloc = 1;
			allocated_footer->splinter = header->splinter;
			/* erase the previous footer*/
			void* erase_foot_ptr = (char*)header + orig_bs*16 - SF_FOOTER_SIZE;
			sf_footer* erase_foot = erase_foot_ptr;

			erase_foot->block_size = 0;
			erase_foot->alloc = 0;
			erase_foot->splinter = 0;

			void* new_link = (char*)allocated_footer_ptr + SF_FOOTER_SIZE;
			freelist_head = new_link;
			freelist_head->next = next;
			freelist_head->prev = prev;
			/* set up new free header*/
			void* new_free_header_ptr = freelist_head;
			sf_header* new_free_header = new_free_header_ptr;

			new_free_header->block_size = temp->header.block_size -
			allocated_footer->block_size + orig_bs;
			/* set up new free footer*/
			void* new_free_footer_ptr = (char*)new_free_header
			+ new_free_header->block_size*16 - SF_FOOTER_SIZE;

			sf_footer* new_free_footer = new_free_footer_ptr;
			new_free_footer->block_size = new_free_header->block_size;
			/* if the header is > end_of_page_adr then freelist is NULL*/
			if(new_free_header_ptr > end_of_page_adr){
				freelist_head = NULL;
			}
			remaining_page_byte += header->block_size*16 - orig_bs*16;

			return (char*)ptr ;
		}

		if(next_free_header->alloc == 1){

			padding += size - orig_size;
			void* best_fit = find_bestfit(orig_size,size);

			//create new free header
			sf_header* free_header = (void*)best_fit;
			int orig_free_block_size = free_header->block_size;

			void* free_header_ptr = (char*)best_fit + size + 16;
			free_header = free_header_ptr;
			if(free_header->block_size - (size+16)/16 == 1){
				// splinter block
				splintering+=16;
				splinterBlocks+=1;
			}
			free_header->block_size = orig_free_block_size - (size/16 + 1);

			//set free_list

			// sort_freelist(free_header);
			sf_free_header* temp = freelist_head;
			 while(temp->next != NULL){
			 	if(temp == best_fit){
			 		break;
			 	}
			 	temp = temp->next;
			 }

			void* next = temp->next;
			void* prev = temp->prev;

			temp = (void*)free_header;
			if(next != temp->next){
				temp->next = next;
			}
			temp->prev = prev;
			freelist_head = temp;

			// create new free footer
			void* free_footer_ptr = (char*)free_header + free_header->block_size*16 - SF_FOOTER_SIZE;
			sf_footer* free_footer = free_footer_ptr;
			free_footer->block_size = free_header->block_size;
			free_footer->alloc = 0;
			free_footer->splinter = free_header->splinter;

			// create new allocated header
			sf_header* allocated_header = best_fit;

			if(allocated_header->block_size - (size+16)/16 == 1){
				allocated_header->splinter = 0;
				allocated_header->splinter_size = 16;

			}
			allocated_header->block_size = (size +16)/16;
			allocated_header->requested_size = size - (size - orig_size);
			allocated_header->alloc = 1;
			allocated_header->padding_size = size - orig_size;

			// create new allocated footer
			void* allocated_footer_ptr = (char*)allocated_header + allocated_header->block_size*16 - 8;
			sf_footer* allocated_footer = allocated_footer_ptr;
			allocated_footer->block_size = allocated_header->block_size;
			allocated_footer->splinter = allocated_header->splinter;
			allocated_footer->alloc = allocated_header->alloc;

			void* best_fit_pl =(char*) best_fit + 8;
			void* ptr_pl = (char*)ptr + 8;
			memcpy(best_fit_pl,ptr_pl,payload_size);

			allocatedBlocks+=1;
			sf_free(ptr);
			//sort_freelist(freelist_head);
			return (char*) allocated_header + 8;
		}

		return NULL;
	}
	else{
		// equal
		return ptr;
	}
	return NULL;
}

bool check_valid_free(void* ptr){
	ptr = (char*)ptr - 8;
	sf_header* h = (void*)ptr;

	if(h->alloc == 1){
		return true;
	}
	else{
		return false;
	}
	return false;
}

bool check_valid_bounds(void* ptr){
	ptr = (char*)ptr - 8;
	if(end_of_page_adr < ptr || original_adr > ptr){
		return false;
	}
	return true;

}
void sf_free(void* ptr) {

	if(ptr == NULL){
		errno = EINVAL;
		return;
	}
	void* ptr_coal = ptr;
	ptr_coal = (char*)ptr_coal + 0;
	bool is_valid = check_valid_free(ptr_coal);
	bool check_bounds =  check_valid_bounds(ptr_coal);
	if(is_valid == false){
		//set error to something
		errno = ENOMEM;
		return;
	}
	if(check_bounds == false){
		errno = ENOMEM;
		return;
	}

	ptr = (char*)ptr - SF_HEADER_SIZE;
	sort_freelist(ptr);

	sf_header *header = ptr;

	padding = padding - header->padding_size;
	if(header->splinter == 1){
		splinterBlocks -= 1;
		splintering -= 16;
	}

	remaining_page_byte -= header->block_size*16;

	payload_size_in_heap -= header->block_size*16 - 16;
	heap_size = (char*)end_of_page_adr - (char*)original_adr;
	peakMemoryUtilization = payload_size_in_heap/heap_size;

	ptr = (char*)ptr + (header->block_size*16) - SF_FOOTER_SIZE;
	sf_footer *footer = ptr;
	//remaining_page_byte += footer->block_size;
	//footer->block_size = header->block_size;
	if(header->splinter == 1){
		footer->splinter = 1;
	}
	else{
		footer->splinter = 0;
	}
	footer->alloc = 0;
	//footer->splinter = 0;

	header->alloc = 0;

	coalescing(ptr_coal);

	allocatedBlocks-=1;
	return;
}
int sf_info(info* ptr) {

	if(ptr == NULL){
		return -1;
	}
	ptr->allocatedBlocks = 0;
	ptr->splinterBlocks = 0;
	ptr->padding = 0;
	ptr->splintering = 0;
	ptr->coalesces = 0;
	ptr->peakMemoryUtilization = 0;

	ptr->allocatedBlocks = allocatedBlocks;
	ptr->splinterBlocks = splinterBlocks;
	ptr->padding = padding;
	ptr->splintering = splintering;
	ptr->coalesces = coalesces;
	ptr->peakMemoryUtilization = peakMemoryUtilization;
	return 0;
}
void* best_fit_equal(int size,sf_free_header *freelist_temp){

	remaining_page_byte -=  size + SF_FOOTER_SIZE + SF_HEADER_SIZE;
	header = setheader(header,size);

	current_adr = freelist_temp;
	freelist_temp->header = header;

	footer_ptr = (char*)footer_ptr + ((header.block_size)*16) ;
	((sf_footer*)footer_ptr)->alloc = 1;
	((sf_footer*)footer_ptr)->block_size = header.block_size;
	((sf_footer*)footer_ptr)->splinter = 0;

	freelist_head = freelist_head->next;
	return (void*)freelist_temp ;
}

void* best_fit_less(int size,sf_free_header* freelist_temp){
	remaining_page_byte -= size + SF_FOOTER_SIZE + SF_HEADER_SIZE;
	header = setheader(header,size);
	freelist_temp->header = header;

	void* updated_head = (char*)freelist_temp + (header.block_size*16);
	void* vp = freelist_head->next;
	void* vp_2 = freelist_head->prev;

	freelist_head = (void*)updated_head;
	freelist_head->next = vp;
	freelist_head->prev = vp_2;

	footer_ptr = (char*)freelist_temp + ((header.block_size)*16) - SF_FOOTER_SIZE ;
	((sf_footer*)footer_ptr)->alloc = 1;
	((sf_footer*)footer_ptr)->block_size = header.block_size;
	((sf_footer*)footer_ptr)->splinter = 0;

	return (void*)freelist_temp;
}

void coal_both(void* h){
	int total_block_size = 0;

	sf_free_header* temp = freelist_head;
	while(temp != NULL){
		if((void*)temp == (void*)h){
			break;
		}
		temp = temp->next;
	}
	temp = temp->prev;

	if(temp->next->next != NULL){
		if(temp->next->next->next != NULL){
			temp->next = temp->next->next->next;
			temp->next->prev = temp;
		}
		else{
			temp->next = NULL;
		}
	}
	else{
		freelist_head->next = NULL;
	}

	// printf("\n");

	void* p = (char*)h;
	sf_header* temp_original_header = p;
	temp_original_header->requested_size = 0;
	temp_original_header->padding_size = 0;
	total_block_size += temp_original_header->block_size ;


	void* nh_p = (char*)temp_original_header + (temp_original_header->block_size*16) ;
	sf_header *next_header = nh_p;

	void* nf_p = (char*)next_header + (next_header->block_size*16) - SF_FOOTER_SIZE;
	sf_footer *next_footer = nf_p;

	total_block_size += next_header->block_size;

	void* pf_p = (char*)temp_original_header- SF_FOOTER_SIZE;
	sf_footer *prev_footer = pf_p;

	void* ph_p = (char*)prev_footer - (prev_footer->block_size*16) + SF_FOOTER_SIZE;
	sf_header *prev_header = ph_p;

	total_block_size += prev_footer->block_size ;

	prev_header->block_size = total_block_size ;
	next_footer->block_size = prev_header->block_size;
}

void coalescing(sf_free_header *h){
	bool prev_ret = prev_alloc((void*)h);
	bool next_ret = next_alloc((void*)h);

	if(prev_ret == false && next_ret == false){
		// check if the block below is free
		void* temp = (char*)h - SF_FOOTER_SIZE;
	
		void* h_ptr = (char*)h - SF_FOOTER_SIZE;
		h = h_ptr;

		coal_both((void*)h);
		coalesces+=1;
		//printf("Prev and Next both not alloced\n");
	}
	else if(prev_ret == true && next_ret == false){
		if((void*)h == original_adr){
		//	printf("at the top of the page\n");
			return;
		}
		void* h_ptr = (char*)h - SF_FOOTER_SIZE;
		h = h_ptr;

		while(freelist_head != NULL){
			if((void*)freelist_head == (void*)h){
				break;
			}
			freelist_head = freelist_head->next;
		}

		//void* temp = (void*)freelist_head;
		if(freelist_head->next->next != NULL){
			freelist_head->next = freelist_head->next->next;
		}
		else{
			freelist_head->next = NULL;
		}

		while(freelist_head->prev != NULL){
			freelist_head = freelist_head->prev;
		}

		sf_header *original_header = (sf_header*)h;
		original_header->requested_size = 0;
		original_header->padding_size = 0;

		void* nh = (char*)original_header + original_header->block_size*16;
		sf_header *next_header = (void*)nh;
		original_header->block_size = next_header->block_size + original_header->block_size;

		void* nf = (char*)next_header + (next_header->block_size*16) - SF_FOOTER_SIZE;
		sf_footer *next_footer = (void*)nf;
		next_footer->block_size = original_header->block_size;


		coalesces+=1;
		//printf("Prev is allocated and Next not alloced\n");
	}
	else if(prev_ret == false && next_ret == true){

		if((char*)h - SF_FOOTER_SIZE == original_adr){
			//printf("at the top of the page\n");
			return;
		}
		void* t_ptr = (char*)h - SF_HEADER_SIZE;
		h = t_ptr;
		sf_free_header* temp = freelist_head;
		while(temp != NULL){
			if((void*)temp == (void*)h){
				break;
			}
			temp = temp->next;
		}

		if(temp->next != NULL){
			temp->prev->next = temp->next;
		}else{
			freelist_head->next = NULL;
		}

		sf_header *original_header = (sf_header*)h;

		original_header->requested_size = 0;
		original_header->padding_size = 0;

		void* o_f = (char*)h + original_header->block_size*16 - SF_FOOTER_SIZE;
		sf_header *original_footer = o_f;
		// Now im at previous footer
		void* move_8 = (char*)h -SF_FOOTER_SIZE;
		h = (void*)move_8;

		sf_footer* prev_footer = (void*)h;

		void* p_h = (char*)prev_footer - prev_footer->block_size*16 + SF_FOOTER_SIZE;
		sf_header *prev_header = p_h;

		prev_header->requested_size = 0;
		prev_header->padding_size = 0;
		prev_header->block_size = prev_header->block_size + original_header->block_size;
		original_footer->block_size = prev_header->block_size;
		coalesces+=1;
	}

	return;
}

bool prev_alloc(void *h){
	void* temp_h = (char*)h - 16;
	h = temp_h;

	sf_footer *footer = (sf_footer*)h;
	if(footer->alloc == 0 && footer->block_size > 0){
		return false;
	}
	else{
		return true;
	}
}

bool next_alloc(void *h){
	void* temp_h = (char*)h - 8 ;
	sf_header *header = temp_h;
	void* temp_h_2 = (char*)h + (header->block_size*16) - SF_FOOTER_SIZE;
	h = temp_h_2;
	header = h;
	if(header->alloc == 0 && header->block_size > 0){
		return false;
	}else{
		return true;
	}
}


sf_header setheader(sf_header header, int size){

		header.requested_size = size;

		int temp = SF_HEADER_SIZE + SF_FOOTER_SIZE + header.requested_size;
		int pad = 0;

		while(temp%16 != 0){
			temp++;
			pad++;
		}
		header.padding_size = pad;
		header.block_size = (temp ) / 16 ;
		header.alloc = 1;
		return header;
}




