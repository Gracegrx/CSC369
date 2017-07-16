#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"

#define MAXBUF 256
extern unsigned memsize;

extern int debug;

extern struct frame *coremap;

static addr_t *frame;
static addr_t *trace;
int pos;
int *record;
int traceLength;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {

	int victim = 0;
	//find the frame with the longest distance, loop through memsize
	int i = 0;
	while (i < memsize)
	{
		int j = pos;
		record[i] = traceLength - pos +1;
		while (j < traceLength)
		{
			if (trace[j] == frame[i])
			{
				if(j-pos+1<record[i])
				{
					record[i] = j-pos+1;
				}
				//printf("record for frame %d = %d", i, record[i]);
				break;
			}
			j++;
		}
		i++;
	}
	
	//find the one with the largest distance in record.	
	int m = 0;
	int temp = 0;
	while (m < memsize)
	{
		if (record[m] == -1)
		{
			victim = m;
			break;
		}
		else if (record[m] > temp)
		{
			temp = record[m];
			victim = m;
		}
		m++;
	}
	//printf("victim = %d\n", victim);
	//printf("traceLength = %d\n", traceLength);
	//printf("pos = %d\n", pos);
	frame[victim] = 0;
	return victim;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {

	//get frame from pte
	int cur;
	cur = (p->frame >> PAGE_SHIFT);
	//add it to memory
	if (!frame[cur])
	{
		frame[cur] = trace[pos];
	}
	pos++;

}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {

	//initialize start position
	pos = 0;
	
	int c;

	// initialize framelist;
	frame = malloc(sizeof(addr_t)*memsize);
	for (c = 0; c < memsize; c++)
	{
		frame[c] = 0;
	}

	//create a list to record distances of frames in memory
	record = malloc(sizeof(int)*memsize);
	for (c = 0; c < memsize; c++)
	{
		record[c] = -1;
	}

	
	// read file
	FILE *traceFile = fopen(tracefile, "r");
	char buf[MAXBUF];

	if (traceFile == NULL)
	{
		perror("Cannot open file");
		exit(1);
	}

	// get the length of tracefile
	//int traceLength = 0;
	while (fgets(buf, MAXBUF, traceFile) != NULL)
	{
		if (buf[0] != '=')
		{
			traceLength++;
		}
	}


	fseek(traceFile, 0, SEEK_SET);

	int i = 0;
	char type;
	addr_t vaddr;

	// record tracefile in a list
	trace = malloc(sizeof(addr_t)*traceLength);
	while (fgets(buf, MAXBUF, traceFile) != NULL)
	{
		if (buf[0] != '=')
		{
			sscanf(buf, "%c %lx", &type, &vaddr);
			trace[i] = vaddr;
			i++;
		}
	}

	fclose(traceFile);



}

