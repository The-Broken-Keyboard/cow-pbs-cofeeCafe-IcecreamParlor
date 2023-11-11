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
struct spinlock no_of_ref;
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
uint no_of_ref_array[PHYSTOP>>12];
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void increase_no_of_ref(void* pa)
{
  acquire(&no_of_ref);
  no_of_ref_array[(uint64)pa>>12]++;
  release(&no_of_ref);
}
void
kinit()
{
  initlock(&no_of_ref,"nfr");
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
      memset((void *)p, 1, PGSIZE);
      struct run *r = (struct run*)p;

      acquire(&kmem.lock);
      r->next = kmem.freelist;
      kmem.freelist = r;
      release(&kmem.lock);

      acquire(&no_of_ref);
        no_of_ref_array[(uint64)p>>12] = 0;
      release(&no_of_ref);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  acquire(&no_of_ref);
  no_of_ref_array[(uint64)pa>>12]--;
  if (no_of_ref_array[(uint64)pa>>12] > 0) {
    release(&no_of_ref);
    return;
  }
  no_of_ref_array[(uint64)pa>>12]=0;
  release(&no_of_ref);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

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

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    acquire(&no_of_ref);
    no_of_ref_array[(uint64)r>>12]=1;
    release(&no_of_ref);
  } 
  return (void*)r;
}
