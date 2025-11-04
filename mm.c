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
  uint64_t user_block[0];   
} BlockHeader;

static BlockHeader * first = NULL;
static BlockHeader * current = NULL;
static BlockHeader * last = NULL;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader *) ((uintptr_t) (p->next) & ~FREE_FLAG_MASK)
#define SET_NEXT(p,n)  do{ \
  uintptr_t current_val = (uintptr_t)((p)->next); \
  uintptr_t free_flag = current_val & FREE_FLAG_MASK; \
  (p)->next = (BlockHeader *) ((uintptr_t) (n) | free_flag); \
}while(0)
#define GET_FREE(p)    (uint8_t) ( (uintptr_t) (p->next) & 0x1 )
#define SET_FREE(p,f)  do{ \
  uintptr_t current_val = (uintptr_t)(p->next); \
  uintptr_t next_ptr = current_val & ~FREE_FLAG_MASK; \
  (p)->next = (BlockHeader *) (next_ptr | ((f) ? FREE_FLAG_MASK : 0)); \
}while(0)
#define ALIGN(size) (((size) + (MIN_SIZE-1)) & ~(MIN_SIZE-1))
#define SIZE(p) ((uintptr_t)GET_NEXT(p) - (uintptr_t)p - sizeof(BlockHeader))
#define MIN_SIZE     (8) 

/**
 * @name  find_previous_block
 * @brief Find the block header for the previous block in the list
 */
/*static BlockHeader* find_previous_block(BlockHeader *block) {
  if(block == NULL || first == NULL) return NULL;
  if(block == first) return last;  // In circular list, last points to first  
  BlockHeader *current_block = first;
  BlockHeader *prev_block = NULL;  
  do {
    BlockHeader *next_block = GET_NEXT(current_block);  // Use GET_NEXT to mask flags    
    SET_FREE(next_block, GET_FREE(current_block)); // Ensure flags are preserved
    if (next_block == block) {
      return current_block;  // Found the previous block
    }    
    prev_block = current_block;
    current_block = next_block;
    if (current_block == first) {
      break;  // We've gone full circle without finding the block
    }    
  } while (current_block != first);  
  return NULL;  // Block not found in the list
}*/

/**
 * @name  coalesce_free_blocks
 * @brief Merge adjacent free blocks to reduce fragmentation
 */
static void coalesce_free_blocks(BlockHeader *block){
  if(block == NULL || GET_FREE(block) == 0 || block == last) return;  
  uint16_t flag = GET_FREE(block);
  BlockHeader * next_block = GET_NEXT(block);
  if(next_block != last && GET_FREE(next_block) == 1){
    SET_NEXT(block, GET_NEXT(next_block));
  }
}


/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 */
void simple_init() {
  uintptr_t aligned_memory_start = (memory_start + MIN_SIZE-1) & ~(MIN_SIZE-1);
  uintptr_t aligned_memory_end   = (memory_end & ~(MIN_SIZE-1));
    
  if (first == NULL) {
    if (aligned_memory_start + 3 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      size_t available_space = aligned_memory_end - aligned_memory_start;
      size_t first_block_size = available_space - 3 * sizeof(BlockHeader);
      
      first = (BlockHeader *) aligned_memory_start;
      last = (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader));
      current = (BlockHeader *) (aligned_memory_end -  2 * sizeof(BlockHeader));

      SET_NEXT(first, current);
      SET_FREE(first, 1);  // First block is FREE
      
      SET_NEXT(current, last);
      SET_FREE(current, 1);  // Dummy is ALLOCATED (never free)
      
      SET_NEXT(last, first);
      SET_FREE(last, 0);   // Last block is ALLOCATED (never free)
    }
    SET_NEXT(last, first);     
    current = first;
  }
}

/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory
 */
void* simple_malloc(size_t size) {
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }
  /*if(GET_FREE(first)==1) {
    current = first;
  }
  coalesce_free_blocks(current);*/
  size_t aligned_size = ALIGN(size); 
  if (aligned_size < MIN_SIZE) aligned_size = MIN_SIZE;
  BlockHeader * search_start = current;
  int iteration = 0;
  do {     
    if (GET_FREE(current) == 1) {
      coalesce_free_blocks(current);
      size_t current_size = SIZE(current);      
      if(current_size >= aligned_size) {        
        if (current_size - aligned_size >= sizeof(BlockHeader) + MIN_SIZE) {
          // Split block
          size_t total_needed = sizeof(BlockHeader) + aligned_size;
          uintptr_t new_block_addr = (uintptr_t)current + total_needed;
          new_block_addr = ALIGN(new_block_addr);
          BlockHeader * new_block = (BlockHeader *) new_block_addr;              
          SET_NEXT(new_block, GET_NEXT(current));
          SET_FREE(new_block, 1);
          SET_NEXT(current, new_block);
          SET_FREE(current, 0);
        } else {
          SET_FREE(current, 0);
        }
        void * result = (void*)current->user_block;
        return result;
      }else {
        printf("Block too small: requested %lu, available %lu\n", aligned_size, current_size);
      }
    }    
    current = GET_NEXT(current);
    iteration++;    
    if (iteration > 100) {
      break;
    }
  } while (current != search_start);
  return NULL;
}

/**
 * @name    simple_free
 * @brief   Frees previously allocated memory
 */
void simple_free(void * ptr) {
  if (ptr == NULL) return;
  BlockHeader * block = (BlockHeader*)((uintptr_t)ptr - sizeof(BlockHeader));  
  if (GET_FREE(block) == 1) {
    return;
  }
  SET_FREE(block, 1);
  coalesce_free_blocks(block);
}

#include "mm_aux.c"