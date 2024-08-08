// ==============================================================================
/**
 * bf-alloc.c
 *
 * A _best-fit_ heap allocator.  This allocator uses a _doubly-linked free list_
 * from which to allocate the best fitting free block.  If the list does not
 * contain any blocks of sufficient size, it uses _pointer bumping_ to expand
 * the heap.
 **/
// ==============================================================================



// ==============================================================================
// INCLUDES

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "safeio.h"
// ==============================================================================



// ==============================================================================
// TYPES AND STRUCTURES

/** The header for each allocated object. */
typedef struct header {

  /** Pointer to the next header in the list. */
  struct header* next;

  /** Pointer to the previous header in the list. */
  struct header* prev;

  /** The usable size of the block (exclusive of the header itself). */
  size_t         size;

  /** Is the block allocated or free? */
  bool           allocated;

} header_s;
// ==============================================================================



// ==============================================================================
// MACRO CONSTANTS AND FUNCTIONS

/** The system's page size. */
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

/**
 * Macros to easily calculate the number of bytes for larger scales (e.g., kilo,
 * mega, gigabytes).
 */
#define KB(size)  ((size_t)size * 1024)
#define MB(size)  (KB(size) * 1024)
#define GB(size)  (MB(size) * 1024)

/** The virtual address space reserved for the heap. */
#define HEAP_SIZE GB(2)

/** Given a pointer to a header, obtain a `void*` pointer to the block itself. */
#define HEADER_TO_BLOCK(hp) ((void*)((intptr_t)hp + sizeof(header_s)))

/** Given a pointer to a block, obtain a `header_s*` pointer to its header. */
#define BLOCK_TO_HEADER(bp) ((header_s*)((intptr_t)bp - sizeof(header_s)))
// ==============================================================================


// ==============================================================================
// GLOBALS

/** The address of the next available byte in the heap region. */
static intptr_t free_addr  = 0;

/** The beginning of the heap. */
static intptr_t start_addr = 0;

/** The end of the heap. */
static intptr_t end_addr   = 0;

/** The head of the free list. */
static header_s* free_list_head = NULL;

/** The head of the allocated list. */ 
static header_s* allocated_list_head = NULL;

// ==============================================================================



// ==============================================================================
/**
 * The initialization method.  If this is the first use of the heap, initialize it.
 */

void init () {

  // Only do anything if there is no heap region (i.e., first time called).
  if (start_addr == 0) {

    DEBUG("Trying to initialize");
    
    // Allocate virtual address space in which the heap will reside. Make it
    // un-shared and not backed by any file (_anonymous_ space).  A failure to
    // map this space is fatal.
    void* heap = mmap(NULL,
		      HEAP_SIZE,
		      PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS,
		      -1,
		      0);
    if (heap == MAP_FAILED) {
      ERROR("Could not mmap() heap region");
    }

    // Hold onto the boundaries of the heap as a whole.
    start_addr = (intptr_t)heap;
    end_addr   = start_addr + HEAP_SIZE;
    free_addr  = start_addr;

    // DEBUG: Emit a message to indicate that this allocator is being called.
    DEBUG("bf-alloc initialized");

  }

} // init ()
// ==============================================================================


// ==============================================================================
/**
 * Allocate and return `size` bytes of heap space.  Specifically, search the
 * free list, choosing the _best fit_.  If no such block is available, expand
 * into the heap region via _pointer bumping_.
 *
 * \param size The number of bytes to allocate.
 * \return A pointer to the allocated block, if successful; `NULL` if unsuccessful.
 */
void* malloc (size_t size) {

  init(); // make sure heap is initialized

  if (size == 0) { // if size is 0 do nothing and return NULL
    return NULL;
  }

  // DEBUG("List head address before malloc call ", (intptr_t)allocated_list_head);
  
  header_s* current = free_list_head; // create a header and set it equal to the current head of the free list
  header_s* best    = NULL;
  while (current != NULL) { // while there are free spaces to consider

    if (current->allocated) {
      ERROR("Allocated block on free list", (intptr_t)current); 
    }
    
    if ( (best == NULL && size <= current->size) ||
	 (best != NULL && size <= current->size && current->size < best->size) ) {
	    // if best is null and block can fit in current space in free list
	    // OR if best is not null but the size can fit and current AND this size is 
	    // smaller than the previous best size
      best = current; // then set the best space to be the current space
    }

    if (best != NULL && best->size == size) { 
      break;
    }

    current = current->next; // set current to be it's next element
    
  }

  void* new_block_ptr = NULL;
  if (best != NULL) { // allocate from best fit block if available otherwise use pointer bumping

    if (best->prev == NULL) {
      free_list_head   = best->next; // if best has no prev, set free list head to its next
    } else {
      best->prev->next = best->next; // set best's prev's next to be best's next
    }
    if (best->next != NULL) {
      best->next->prev = best->prev; // if best's next != NULL, set its prev to be best's prev
    }
    best->prev = NULL; // set best's prev to be NULL

    // my code
    best->next = allocated_list_head;
    if(allocated_list_head != NULL) {
    	allocated_list_head->prev = best; // update allocated list head to be best
    }
    allocated_list_head = best;
    // end of my code

    best->allocated = true; // set the blocks allocated field to be true
    new_block_ptr   = HEADER_TO_BLOCK(best); // update block pointer
    
  } else { // no suitable block found in free list, use pointer bumping

    int modVal = free_addr % 16; // calc the mod of free_addr
    if (modVal != 0) { // if current alignment is < 8, adjust it to next 8 byte
	  free_addr += (16-modVal);
    }

    header_s* header_ptr = (header_s*)free_addr; // create a new block at the current free
    new_block_ptr = HEADER_TO_BLOCK(header_ptr);

    header_ptr->next = allocated_list_head; // update header ptr's next to be allocated list head
    if (header_ptr->next != NULL) {
	    allocated_list_head->prev = header_ptr; // set allocated list head prev to be header ptr
    }
    allocated_list_head = header_ptr; // set allocated list head to be header ptr

    // update block metadata
    header_ptr->prev      = NULL;
    header_ptr->size      = size;
    header_ptr->allocated = true;
    
    // update free addy for the next allocation
    intptr_t new_free_addr = (intptr_t)new_block_ptr + size;
    if (new_free_addr > end_addr) {
      return NULL; // heap boundary reached
    } else {
      free_addr = new_free_addr; // else set free addy to be new free addy
    }
  }

  /*
  // printing my debug info 
  header_s* tmp = allocated_list_head;
  DEBUG("List head address after malloc call ", (intptr_t) allocated_list_head);
  while (tmp != NULL && printDebug) { // this while loop prints all the blocks in allocated list
	DEBUG("Allocated block address: ", (intptr_t) HEADER_TO_BLOCK(tmp));
	tmp = tmp->next;
  }  
  */
 

  return new_block_ptr; // return addy of the new block

} // malloc()
// ==============================================================================



// ==============================================================================
/**
 * Deallocate a given block on the heap.  Add the given block (if any) to the
 * free list.
 *
 * \param ptr A pointer to the block to be deallocated.
 */
void free (void* ptr) {

  if (ptr == NULL) {
    return; // nothing to free so return NULL
  }

  //DEBUG("List head address BEFORE FREE call ", (intptr_t) allocated_list_head);

  header_s* header_ptr = BLOCK_TO_HEADER(ptr); // retrieve header of block to be freed

  if (!header_ptr->allocated) {
    ERROR("Double-free: ", (intptr_t)header_ptr); // if header's field allocated is free then return an error stating they tried to double free a block
  }

  if (header_ptr->prev == NULL) { // check if block to be freed is at beginning of allocated list
      allocated_list_head = header_ptr->next; // set allocate list head to be its next
  } 
  else {
      header_ptr->prev->next = header_ptr->next; // block to be freed is not at the beginning so update the previous block's next ptr
  }
  if (header_ptr->next != NULL) { // update the prev ptr of the next block in the allocated list
      header_ptr->next->prev = header_ptr->prev;
  }
  header_ptr->prev = NULL; // clear the prev ptr of the block being freed

  header_ptr->next = free_list_head; // set the next ptr of the free block to return the current head of the free list
  free_list_head   = header_ptr; // update the head of the free list to point to the freed block
  header_ptr->prev = NULL; // clear prev pointer of freed block
  if (header_ptr->next != NULL) { // if free list is not empty update prev ptr
    header_ptr->next->prev = header_ptr;
  }
  header_ptr->allocated = false; // set field of block to be un-allocated

  /*
  // my debug info
  header_s* tmp = allocated_list_head;
  DEBUG("List head address AFTER FREE call ", (intptr_t) allocated_list_head);
  while (tmp != NULL && printDebug) { // print the allocated list 
	DEBUG("Allocated block addresses: ", (intptr_t) HEADER_TO_BLOCK(tmp));
	tmp = tmp->next;
  }
  */

}
// free()
// ==============================================================================




// ==============================================================================
/**
 * Allocate a block of `nmemb * size` bytes on the heap, zeroing its contents.
 *
 * \param nmemb The number of elements in the new block.
 * \param size  The size, in bytes, of each of the `nmemb` elements.
 * \return      A pointer to the newly allocated and zeroed block, if successful;
 *              `NULL` if unsuccessful.
 */
void* calloc (size_t nmemb, size_t size) {

  // Allocate a block of the requested size.
  size_t block_size    = nmemb * size;
  void*  new_block_ptr = malloc(block_size);

  // If the allocation succeeded, clear the entire block.
  if (new_block_ptr != NULL) {
    memset(new_block_ptr, 0, block_size);
  }

  return new_block_ptr;
  
} // calloc ()
// ==============================================================================



// ==============================================================================
/**
 * Update the given block at `ptr` to take on the given `size`.  Here, if `size`
 * fits within the given block, then the block is returned unchanged.  If the
 * `size` is an increase for the block, then a new and larger block is
 * allocated, and the data from the old block is copied, the old block freed,
 * and the new block returned.
 *
 * \param ptr  The block to be assigned a new size.
 * \param size The new size that the block should assume.
 * \return     A pointer to the resultant block, which may be `ptr` itself, or
 *             may be a newly allocated block.
 */
void* realloc (void* ptr, size_t size) {

  // Special case: If there is no original block, then just allocate the new one
  // of the given size.
  if (ptr == NULL) {
    return malloc(size);
  }

  // Special case: If the new size is 0, that's tantamount to freeing the block.
  if (size == 0) {
    free(ptr);
    return NULL;
  }

  // Get the current block size from its header.
  header_s* header_ptr = BLOCK_TO_HEADER(ptr);

  // If the new size isn't an increase, then just return the original block as-is.
  if (size <= header_ptr->size) {
    return ptr;
  }

  // The new size is an increase.  Allocate the new, larger block, copy the
  // contents of the old into it, and free the old.
  void* new_block_ptr = malloc(size);
  if (new_block_ptr != NULL) {
    memcpy(new_block_ptr, ptr, header_ptr->size);
    free(ptr);
  }
    
  return new_block_ptr;
  
} // realloc()
// ==============================================================================


