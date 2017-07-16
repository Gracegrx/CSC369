#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int count;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {

	int victim = 0;
	int oldest = count;
	int i;
	for (i = 0; i < memsize; i++)
	{
		if (coremap[i].pte->timeStamp < oldest)
		{
			oldest = coremap[i].pte->timeStamp;
			victim = i;
		}
	}
	//printf("victim = %d", victim);
	return victim;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	p->timeStamp = count;
	count++;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	count = 0;
}
