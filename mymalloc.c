#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h>

// Include any other headers we need here

// NOTE: You should NOT include <stdlib.h> in your final implementation.
// malloc.h turns every call to malloc/calloc/free in this file into a call
// to mymalloc/mycalloc/myfree, so calling them here would recurse forever.

#include <debug.h> // definition of debug_printf

void *mymalloc(size_t s) {

  void *p = NULL; // TODO: find the first free block that fits, or request a
                  // new one with sbrk. Determine how you will request memory :)

  if (!p) {
    // We are out of memory
    // if sbrk fails (returns (void *) -1)
  }
  debug_printf("Malloc %zu bytes\n", s);

  return p;
}

void *mycalloc(size_t nmemb, size_t s) {

  void *p = NULL; // TODO: get the memory from mymalloc, then set it to 0

  if (!p) {
    // We are out of memory
    // if we get NULL back from mymalloc
  }
  debug_printf("Calloc %zu bytes\n", nmemb * s);

  return p;
}

void myfree(void *ptr) {
  size_t size = 0; // TODO: the size stored in ptr's block header

  // TODO: mark ptr's block as free

  debug_printf("Freed %zu bytes\n", size);
}
