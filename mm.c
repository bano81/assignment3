/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>
#include "mm.h"
// Define the block header structure for circular linked list
typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

static BlockHeader * first = NULL;
static BlockHeader * current = NULL;
static BlockHeader * last = NULL;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader *) ((uintptr_t) (p->next) & ~FREE_FLAG_MASK)  /* Mask out free flag */
#define SET_NEXT(p,n)  do{ \
  uintptr_t current_val = (uintptr_t)((p)->next); \
  uintptr_t free_flag = current_val & FREE_FLAG_MASK; \
  (p)->next = (BlockHeader *) ((uintptr_t) (n) | free_flag); \
}while(0)
#define GET_FREE(p)    (uint8_t) ( (uintptr_t) (p->next) & 0x1 )   /* OK -- do not change */
#define SET_FREE(p,f)  do{ \
  uintptr_t current_val = (uintptr_t)(p->next); \
  uintptr_t next_ptr = current_val & ~FREE_FLAG_MASK; \
  (p)->next = (BlockHeader *) (next_ptr | ((f) ? FREE_FLAG_MASK : 0)); \
}while(0)
#define ALIGN(size) (((size) + (MIN_SIZE-1)) & ~(MIN_SIZE-1)) /* Align size to multiple of MIN_SIZE */
#define SIZE(p)  (size_t)((uintptr_t)GET_NEXT(p)-(uintptr_t)p-sizeof(BlockHeader)) /* Caluculate size of block from p and p->next */
#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user



/**
 * @name  find_previous_block
 * @brief Find the block header for the previous block in the list
 *
 * @param BlockHeader *block Pointer to the current block
 * @retval BlockHeader* Pointer to the previous block or NULL if not found
 */
static BlockHeader* find_previous_block(BlockHeader *block) {
  BlockHeader *current_block = first;
  BlockHeader *prev = NULL;
  do{
    BlockHeader *next_block = GET_NEXT(current_block);
    if(next_block == block) return current_block;
    prev = current_block;
    current_block = next_block;
  } while(current_block != first);
  return NULL;
}

/**
 * @name  coalesce_free_blocks
 * @brief Merge adjacent free blocks to reduce fragmentation
 * 
 * @param BlockHeader *block Pointer to the current block
 */
static void coalesce_free_blocks(BlockHeader *block){
  BlockHeader * next_block = GET_NEXT(block); 
  if(GET_FREE(next_block) && next_block != last){
    SET_NEXT(block, GET_NEXT(next_block)); // Use SET_NEXT to preserve flag
  }
  BlockHeader *prev = find_previous_block(block);
  if(prev != NULL && GET_FREE(prev) && prev!=last){
    SET_NEXT(prev, GET_NEXT(block)); // Use SET_NEXT to preserve flag
  }
}


/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
  uintptr_t aligned_memory_start = (memory_start + MIN_SIZE-1) & ~(MIN_SIZE-1); // To insure 8 bits alignment, which makes is faster on most architectures
  uintptr_t aligned_memory_end   = (memory_end & ~(MIN_SIZE-1));    // To insure 8 bits alignment, which makes is faster on most architectures
  BlockHeader * dummy; //
  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      /* TODO: Place first and last blocks and set links and free flags properly */
      first = (BlockHeader *) aligned_memory_start;
      size_t first_block_size = aligned_memory_end - aligned_memory_start - sizeof(BlockHeader);
      dummy = (BlockHeader *) (aligned_memory_end - sizeof(BlockHeader)); // End block (circular list terminator)
      SET_NEXT(first, dummy);
      SET_FREE(first, 1); // Free block
      SET_NEXT(dummy, first); // Set up circular list
      SET_FREE(dummy, 0); // Last block is never free
      last = dummy;
    }
    current = first;     
  }
}


/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
  
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }

  size_t aligned_size = ALIGN(size); 
  if (aligned_size < MIN_SIZE) aligned_size = MIN_SIZE;

  /* Search for a free block */
  BlockHeader * search_start = current;
  do { 
    if (GET_FREE(current)) {
      /* Possibly coalesce consecutive free blocks here */
      coalesce_free_blocks(current);
      /* Check if free block is large enough */
      size_t current_size = SIZE(current);
      if(current_size >= aligned_size) {
        /* Found a block large enough */
        if (current_size - aligned_size < sizeof(BlockHeader) + MIN_SIZE){
          SET_FREE(current, 0);
        } else { // Split block
          BlockHeader *new_block = (BlockHeader *)((uintptr_t)current +sizeof(BlockHeader) + aligned_size);
          SET_NEXT(new_block, GET_NEXT(current)); // new_block points to the current's next 
          SET_FREE(new_block, 1);  //set up new free block
          SET_NEXT(current, new_block); // update current block to allocated size
          SET_FREE(current,0); // mark current as allocated
        }
        void * result = (void*)current->user_block; // Return pointer to user block
        current = GET_NEXT(current); // Update current for next search
        return result;
      }
    }
    current = GET_NEXT(current); // Move to next block
  } while (current != search_start);
  return NULL; // No suitable block found
}


/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
  if (ptr == NULL) return; // Nothing to free
  BlockHeader * block = (BlockHeader*)((uintptr_t)ptr - sizeof(BlockHeader)); // Get block header
  if (GET_FREE(block)) {
    return; // Block is already free, do nothing
  }
  SET_FREE(block, 1); // Mark block as free
  coalesce_free_blocks(block); // Coalesce adjacent free blocks
}

#include "mm_aux.c"
