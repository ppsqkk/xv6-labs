// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

// All struct run *s are physical addresses
struct run {
  struct run *next;
};

// Initially, kmem.freelist == NULL
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  // Paging disabled
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

// Only used to initialize the free list
// pa stands for physical address
// pa_end is a physical address that cannot be part of a page
void
freerange(void *pa_start, void *pa_end)
{
  char *p;

  // Assume pa_start + 4096 - 1 doesn't overflow
  p = (char*)PGROUNDUP((uint64)pa_start);

  // Invariant: p is aligned on a page boundary
  // p + PGSIZE is 1 byte off the end of the current page
  // The condition is equivalent to p + PGSIZE - 1 < (char *)pa_end,
  // which means that the last byte of the page is not pa_end
  // Invariant: pa_end is never a part of a page
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  // Physical address of page must be...
  // 1. aligned on a page boundary
  // 2. >= end
  // 3. < PHYSTOP (can't have a page starting at PHYSTOP)
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  // Place a struct run at the beginning of the free page
  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r; // Return a physical address
}
