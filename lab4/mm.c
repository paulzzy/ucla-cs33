/*
 * mm.c -  Simple allocator based on implicit free lists,
 *         first fit placement, and boundary tag coalescing.
 *
 * Each block has header and footer of the form:
 *
 *      63       32   31        1   0
 *      --------------------------------
 *     |   unused   | block_size | a/f |
 *      --------------------------------
 *
 * a/f is 1 iff the block is allocated. The list has the following form:
 *
 * begin                                       end
 * heap                                       heap
 *  ----------------------------------------------
 * | hdr(8:a) | zero or more usr blks | hdr(0:a) |
 *  ----------------------------------------------
 * | prologue |                       | epilogue |
 * | block    |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */
#include "mm.h"
#include "memlib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG_OUTPUT

/* Your info */
team_t team = {
    /* First and last name */
    "implicit first fit",
    /* UID */
    "123456789",
    /* Custom message (16 chars) */
    "",
};

typedef struct {
  uint32_t allocated : 1;
  uint32_t block_size : 31;
  uint32_t _;
} header_t;

typedef header_t footer_t;

typedef struct block_t {
  uint32_t allocated : 1;
  uint32_t block_size : 31;
  uint32_t _;
  union {
    struct {
      struct block_t *next;
      struct block_t *prev;
    };
    int payload[0];
  } body;
} block_t;

/* This enum can be used to set the allocated bit in the block */
enum block_state { FREE, ALLOC };

#define CHUNKSIZE (1 << 16) /* initial heap size (bytes) */
#define OVERHEAD                                                               \
  (sizeof(header_t) + sizeof(footer_t)) /* overhead of the header and footer   \
                                           of an allocated block */
#define MIN_BLOCK_SIZE                                                         \
  (32) /* the minimum block size needed to keep in a freelist (header + footer \
          + next pointer + prev pointer) */

/* Global variables */
static block_t *prologue; /* pointer to first block */
static block_t *head;     // Head pointer of explicit free list (null-terminated
                          // doubly-linked list)

// Debug variables
static int global_counter = 1;
static const int LIST_DEPTH = 1000;

#ifndef DEBUG_OUTPUT

#define CHECK_EXPLICIT_LIST(list_depth)
#define CHECK_IN_LIST(block)

#endif

// Debug macros
#ifdef DEBUG_OUTPUT

#define CHECK_EXPLICIT_LIST(list_depth) debug_explicit_list(list_depth)
#define CHECK_IN_LIST(block) debug_check_in_list(block)

#endif

/* function prototypes for internal helper routines */
// Explicit free list functions
static void list_push(block_t *block);
static void list_remove(block_t *block);

// Debugging functions
static void debug_explicit_list(int depth);
static void debug_check_in_list(block_t *block);

// Original functions given by instructor
static void mm_checkheap(int verbose);
static block_t *extend_heap(size_t words);
static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);
static footer_t *get_footer(block_t *block);
static void printblock(block_t *block);
static void checkblock(block_t *block);

/*
 * mm_init - Initialize the memory manager
 */
/* $begin mminit */
int mm_init(void) {
  /* create the initial empty heap */
  if ((prologue = mem_sbrk(CHUNKSIZE)) == (void *)-1) {
    return -1;
  }
  /* initialize the prologue */
  prologue->allocated = ALLOC;
  prologue->block_size = sizeof(header_t);

  /* initialize the first free block */
  block_t *init_block = (void *)prologue + sizeof(header_t);
  init_block->allocated = FREE;
  init_block->block_size = CHUNKSIZE - OVERHEAD;
  footer_t *init_footer = get_footer(init_block);
  init_footer->allocated = FREE;
  init_footer->block_size = init_block->block_size;

  // Initialize the explicit free list (uses a doubly-linked list)
  // prologue <-> init_block
  prologue->body.next = init_block;
  prologue->body.prev = init_block;
  init_block->body.next = prologue;
  init_block->body.prev = prologue;

  /* initialize the epilogue - block size 0 will be used as a terminating
   * condition */
  block_t *epilogue = (void *)init_block + init_block->block_size;
  epilogue->allocated = ALLOC;
  epilogue->block_size = 0;
  return 0;
}
/* $end mminit */

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size) {
  uint32_t asize = 0;       /* adjusted block size */
  uint32_t extendsize = 0;  /* amount to extend heap if no fit */
  uint32_t extendwords = 0; /* number of words to extend heap if no fit */
  block_t *block = NULL;

  /* Ignore spurious requests */
  if (size == 0) {
    return NULL;
  }

  /* Adjust block size to include overhead and alignment reqs. */
  size += OVERHEAD;

  asize = ((size + 7) >> 3) << 3; /* align to multiple of 8 */

  if (asize < MIN_BLOCK_SIZE) {
    asize = MIN_BLOCK_SIZE;
  }

  /* Search the free list for a fit */
  if ((block = find_fit(asize)) != NULL) {
    place(block, asize);
    return block->body.payload;
  }

  /* No fit found. Get more memory and place the block */
  extendsize = (asize > CHUNKSIZE) // extend by the larger of the two
                   ? asize
                   : CHUNKSIZE;
  extendwords = extendsize >> 3; // extendsize/8
  if ((block = extend_heap(extendwords)) != NULL) {
    place(block, asize);
    return block->body.payload;
  }
  /* no more memory :( */
  return NULL;
}
/* $end mmmalloc */

/*
 * mm_free - Free a block
 */
/* $begin mmfree */
void mm_free(void *payload) {
  block_t *block = payload - sizeof(header_t);
  block->allocated = FREE;
  footer_t *footer = get_footer(block);
  footer->allocated = FREE;
  coalesce(block);
}

/* $end mmfree */

/*
 * mm_realloc - naive implementation of mm_realloc
 * NO NEED TO CHANGE THIS CODE!
 */
void *mm_realloc(void *ptr, size_t size) {
  void *newp = NULL;
  size_t copySize = 0;

  if ((newp = mm_malloc(size)) == NULL) {
    printf("ERROR: mm_malloc failed in mm_realloc\n");
    exit(1);
  }
  block_t *block = ptr - sizeof(header_t);
  copySize = block->block_size;
  if (size < copySize) {
    copySize = size;
  }
  memcpy(newp, ptr, copySize);
  mm_free(ptr);
  return newp;
}

/*
 * mm_checkheap - Check the heap for consistency
 */
void mm_checkheap(int verbose) {
  block_t *block = prologue;

  if (verbose) {
    printf("Heap (%p):\n", prologue);
  }

  if (block->block_size != sizeof(header_t) || !block->allocated) {
    printf("Bad prologue header\n");
  }
  checkblock(prologue);

  /* iterate through the heap (both free and allocated blocks will be present)
   */
  for (block = (void *)prologue + prologue->block_size; block->block_size > 0;
       block = (void *)block + block->block_size) {
    if (verbose) {
      printblock(block);
    }
    checkblock(block);
  }

  if (verbose) {
    printblock(block);
  }
  if (block->block_size != 0 || !block->allocated) {
    printf("Bad epilogue header\n");
  }
}

/* The remaining routines are internal helper routines */

// Pushes a block to the front of the explicit free list
//
// Warning: Only use to push free blocks
static void list_push(block_t *block) {

#ifdef DEBUG_OUTPUT
  printf("\nDEBUG LIST_PUSH: %d\n", global_counter);
  printf("head: %p\n", head);
  printf("block: %p\n", block);
  global_counter++;

  CHECK_IN_LIST(block);
  CHECK_EXPLICIT_LIST(LIST_DEPTH);
#endif

  // Zero elements
  if (head == NULL) {
    head = block;
    block->body.next = NULL;
    block->body.prev = NULL;
    return;
  }

  block->body.next = head;
  block->body.prev = NULL;

  head->body.prev = block;

  head = block;
}

// Removes a block from the explicit free list
//
// Warning: Assumes block is in list
// Warning: Only use to remove allocated blocks
static void list_remove(block_t *block) {

#ifdef DEBUG_OUTPUT
  printf("\nDEBUG LIST_REMOVE: %d\n", global_counter);
  printf("head: %p\n", head);
  printf("following: %p\n", block->body.next);
  printf("preceding: %p\n", block->body.prev);
  global_counter++;

  CHECK_EXPLICIT_LIST(LIST_DEPTH);
#endif

  if (head == NULL || block == NULL) {
    return;
  }

  // One-element list
  if (head->body.next == NULL) {
    head = NULL;
    return;
  }

  // Removal block is head
  if (head == block) {
    head = block->body.next;
    head->body.prev = NULL;
  }

  block_t *preceding = block->body.prev;
  block_t *following = block->body.next;

  if (preceding != NULL) {
    preceding->body.next = block->body.next;
  }

  if (following != NULL) {
    following->body.prev = block->body.prev;
  }

  block->body.next = NULL;
  block->body.prev = NULL;
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static block_t *find_fit(size_t asize) {
  /* first fit search */
  block_t *b = NULL;

  for (b = (void *)prologue + prologue->block_size; b->block_size > 0;
       b = (void *)b + b->block_size) {
    /* block must be free and the size must be large enough to hold the request
     */
    if (!b->allocated && asize <= b->block_size) {
      return b;
    }
  }
  return NULL; /* no fit */
}

/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block
 */
static block_t *coalesce(block_t *block) {
  footer_t *prev_footer = (void *)block - sizeof(header_t);
  header_t *next_header = (void *)block + block->block_size;
  bool prev_alloc = prev_footer->allocated;
  bool next_alloc = next_header->allocated;

  if (prev_alloc && next_alloc) { /* Case 1 */
    /* no coalesceing */
    return block;
  }

  if (prev_alloc && !next_alloc) { /* Case 2 */
    /* Update header of current block to include next block's size */
    block->block_size += next_header->block_size;
    /* Update footer of next block to reflect new size */
    footer_t *next_footer = get_footer(block);
    next_footer->block_size = block->block_size;
  } else if (!prev_alloc && next_alloc) { /* Case 3 */
    /* Update header of prev block to include current block's size */
    block_t *prev_block =
        (void *)prev_footer - prev_footer->block_size + sizeof(header_t);
    prev_block->block_size += block->block_size;
    /* Update footer of current block to reflect new size */
    footer_t *footer = get_footer(prev_block);
    footer->block_size = prev_block->block_size;
    block = prev_block;
  } else { /* Case 4 */
    /* Update header of prev block to include current and next block's size */
    block_t *prev_block =
        (void *)prev_footer - prev_footer->block_size + sizeof(header_t);
    prev_block->block_size += block->block_size + next_header->block_size;
    /* Update footer of next block to reflect new size */
    footer_t *next_footer = get_footer(prev_block);
    next_footer->block_size = prev_block->block_size;
    block = prev_block;
  }

  return block;
}

static footer_t *get_footer(block_t *block) {
  return (void *)block + block->block_size - sizeof(footer_t);
}

static void printblock(block_t *block) {
  uint32_t hsize = 0;
  uint32_t halloc = 0;
  uint32_t fsize = 0;
  uint32_t falloc = 0;

  hsize = block->block_size;
  halloc = block->allocated;
  footer_t *footer = get_footer(block);
  fsize = footer->block_size;
  falloc = footer->allocated;

  if (hsize == 0) {
    printf("%p: EOL\n", block);
    return;
  }

  printf("%p: header: [%d:%c] footer: [%d:%c]\n", block, hsize,
         (halloc ? 'a' : 'f'), fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(block_t *block) {
  if ((uint64_t)block->body.payload % 8) {
    printf("Error: payload for block at %p is not aligned\n", block);
  }
  footer_t *footer = get_footer(block);
  if (block->block_size != footer->block_size) {
    printf("Error: header does not match footer\n");
  }
}

static void debug_explicit_list(int depth) {
  printf("\nDEBUG EXPLICIT LIST: %d\n", global_counter);
  global_counter++;

  if (head == NULL) {
    printf("0 elements.\n");
    return;
  }

  int f_len = 0;
  int b_len = 0;

  // Traverse forward.
  block_t *forward = head;
  int f_idx = 0;

  for (; f_idx < depth; f_idx++) {
    if (forward->body.next == NULL) {
      printf("%p (tail)\n", forward);
      f_len++;
      printf("  Forward traversal: %d elements.\n", f_len);
      break;
    }

    printf("%p -> ", forward);
    forward = forward->body.next;
    f_len++;
  }

  if (f_idx == depth) {
    printf("\nWARNING: Reached forward depth limit.\n");
  }

  // Traverse backwards.
  block_t *backward = forward;
  int b_idx = 0;

  for (; b_idx < depth; b_idx++) {
    if (backward->body.prev == NULL) {
      printf("%p (head)\n", backward);
      b_len++;
      printf("  Backward traversal: %d elements.\n", b_len);
      break;
    }

    printf("%p -> ", backward);
    backward = backward->body.prev;
    b_len++;
  }

  if (b_idx == depth) {
    printf("\nWARNING: Reached backward depth limit.\n");
  }

  if (f_len != b_len) {
    printf("ERROR: length mismatch for forward and backward traversal.\n");
    exit(1);
  } else {
    printf("Validated: equal lengths for forward and backward traversal.\n");
  }
}

static void debug_check_in_list(block_t *block) {
  printf("\nDEBUG CHECK IN LIST: %d\n", global_counter);
  global_counter++;

  if (head == NULL) {
    return;
  }

  int list_idx = 0;
  for (block_t *current = head; current != NULL; current = current->body.next) {
    if (block == current) {
      printf("ERROR: block %p already in list at index %d\n", block, list_idx);
      exit(1);
      return;
    }

    list_idx++;
  }

  printf("Validated: block %p.\n", block);
}
