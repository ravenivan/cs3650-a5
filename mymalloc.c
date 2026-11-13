/*
 * mymalloc.c - A basic first-fit memory allocator built on sbrk.
 *
 * Design:
 *   Every chunk obtained from sbrk is laid out as a header (block_t)
 *   immediately followed by the user's memory:
 *
 *     |block_t|---user memory---|block_t|---user memory---| ...
 *
 *   All blocks, free and in use, are kept in a single singly linked list
 *   in the order they were obtained from sbrk. New blocks are appended at
 *   the tail. Allocation walks the list from the head and reuses the first
 *   free block that is large enough (first fit). sbrk is called only when
 *   no free block fits, and it requests exactly sizeof(block_t) + size.
 *
 * Assumptions:
 *   - Single-threaded use (no locking).
 *   - The heap is never shrunk; freed blocks are only marked free.
 *   - No splitting or coalescing, so a reused block may be larger than
 *     the request.
 *   - No alignment beyond what sbrk provides.
 */

#define _DEFAULT_SOURCE // expose sbrk in <unistd.h>
#define _BSD_SOURCE

// System headers come first so malloc.h's aliases can't affect them.
// <stdlib.h> is intentionally not included (see assignment rules).
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <debug.h>  // debug_printf
#include <malloc.h> // mymalloc/mycalloc/myfree prototypes and aliases

/* Header stored directly before each block of user memory. */
typedef struct block {
  size_t size;        // bytes of user memory following this header
  struct block *next; // next block in sbrk order (free or in use)
  int free;           // 1 if the block is available for reuse, else 0
} block_t;

#define HEADER_SIZE sizeof(block_t)

static block_t *head = NULL; // first block obtained from sbrk
static block_t *tail = NULL; // last block, so appends are O(1)

/*
 * Returns the header of the block whose user memory starts at ptr.
 * ptr must be a non-NULL pointer previously returned by mymalloc.
 */
static block_t *header_of(void *ptr) {
  assert(ptr != NULL);
  return ((block_t *)ptr) - 1;
}

/* Returns a pointer to the user memory that follows header b. */
static void *payload_of(block_t *b) {
  assert(b != NULL);
  return (void *)(b + 1);
}

/*
 * Walks the block list from the head and returns the first free block
 * with at least size bytes of user memory, or NULL if there is none.
 */
static block_t *find_free_block(size_t size) {
  for (block_t *cur = head; cur != NULL; cur = cur->next) {
    if (cur->free && cur->size >= size) {
      return cur;
    }
  }
  return NULL;
}

/* Appends block b to the tail of the block list. */
static void append_block(block_t *b) {
  assert(b != NULL && b->next == NULL);
  if (head == NULL) {
    assert(tail == NULL);
    head = b;
  } else {
    assert(tail != NULL && tail->next == NULL);
    tail->next = b;
  }
  tail = b;
}

/*
 * Grows the heap with sbrk by exactly HEADER_SIZE + size bytes,
 * initializes the new block as in use, and appends it to the list.
 * Returns the new block, or NULL if the request is too large or sbrk fails.
 */
static block_t *request_block(size_t size) {
  // sbrk takes a signed intptr_t, so reject sizes that would overflow it.
  if (size > (size_t)INTPTR_MAX - HEADER_SIZE) {
    return NULL;
  }
  void *mem = sbrk((intptr_t)(HEADER_SIZE + size));
  if (mem == (void *)-1) {
    return NULL;
  }
  block_t *b = mem;
  b->size = size;
  b->next = NULL;
  b->free = 0;
  append_block(b);
  return b;
}

/*
 * Allocates size bytes of uninitialized memory.
 * Reuses the first free block that fits; otherwise grows the heap.
 * Returns a pointer to the memory, or NULL if size is 0 or sbrk fails.
 */
void *mymalloc(size_t size) {
  if (size == 0) {
    return NULL;
  }
  debug_printf("Malloc %zu bytes\n", size);

  block_t *b = find_free_block(size);
  if (b != NULL) {
    b->free = 0;
  } else {
    b = request_block(size);
    if (b == NULL) {
      return NULL;
    }
  }
  assert(!b->free && b->size >= size);
  return payload_of(b);
}

/*
 * Allocates zeroed memory for an array of nmemb elements of size bytes.
 * Gets its memory from mymalloc. Returns NULL if nmemb * size overflows,
 * is 0, or the allocation fails.
 */
void *mycalloc(size_t nmemb, size_t size) {
  if (size != 0 && nmemb > SIZE_MAX / size) {
    return NULL; // nmemb * size would overflow
  }
  size_t total = nmemb * size;
  void *ptr = mymalloc(total);
  if (ptr == NULL) {
    return NULL;
  }
  debug_printf("Calloc %zu bytes\n", total);
  memset(ptr, 0, total);
  return ptr;
}

/*
 * Marks the block whose memory starts at ptr as free so it can be reused.
 * Does nothing if ptr is NULL. ptr must come from mymalloc/mycalloc and
 * must not already be freed.
 */
void myfree(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  block_t *b = header_of(ptr);
  assert(!b->free); // catches double frees
  b->free = 1;
  debug_printf("Freed %zu bytes\n", b->size);
}
