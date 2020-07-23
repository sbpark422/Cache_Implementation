/*
 * cache.c
 *
 * 20493-02 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 13, 2017
 *
 */
	

#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

/* get the variables in other files */
extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

// cache with size (# of sets X # of associations), 4X1 for direct, 2X2 fpr 2-way, 1X4 for fully
cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];

int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)  i = 0;	 // cycle
        if (++j >= 16)  j = 0;	 // cycle
         
        if (i == 0 && j == i+gap)	 // differences of i and j == gap
            j = i + (++gap);	 // increases 1 gap and new j for each cycle
            
		printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
	printf("\n\n");
}   

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
	/* initialize cache data */
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;		// invalid 
            pEntry->tag = -1;		// no tag
            pEntry->timestamp = 0;	// no access trial
        }
    }
}

/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
	printf("ENTRY >> \n");
	// for each set
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d]\n", i);
		// for each entry in a set
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("   V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            // for each data[] in a entry
			for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}
/* This function is to return the data in cache */
int check_cache_data_hit(void *addr, char type) {
	int j; 
	int set_index;
	int tag;
	int block_addr;
	int block_offset;
	int access=0;
	
	/* add cache access cycle to global access cycle */
	num_access_cycles += CACHE_ACCESS_CYCLE;
	
	//calculating block address, block offset, set index, and tag
	block_addr= *(int*)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	block_offset = *(int*)addr % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	set_index = block_addr % CACHE_SET_SIZE;
	tag = block_addr / CACHE_SET_SIZE;

	/* check all entries in a set */ 
	for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[set_index][j];
			if(pEntry->valid==1 && pEntry->tag==tag){
				access =1;  // if valid bit is present and same tag, hit 
				pEntry->timestamp = global_timestamp; // update timestamp of this entry 
				num_cache_hits++; // increase hit event

				if(type =='b') // return the cache data with a suitable type (b, h, or w) 
					return *((char*)&pEntry->data[block_offset]);
				else if (type =='h')
					return *((short*)&pEntry->data[block_offset]);
				else if (type=='w')
					return *((int*)&pEntry->data[block_offset]);
			}
	}

	/* if there is no desired data in cache, missed and return -1*/
	if(access==0){
		num_cache_misses++; // increase num_cache_misses
		return -1;
	}

	//return -1 for missing in cache
    return-1; 
}
/* This function is to find the entry index in set for copying to cache */
int find_entry_index_in_set(int cache_index) {
    int entry_index;
	int j;
	int min_timestamp = 1000;// initialize min_timestamp to very large number
				             // Because mintime must always be bigger at first 
				             // to find the value accessed the longest ago.
   
	/* Check if there exists any empty cache space by checking 'valid' */
	 for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[cache_index][j];
			// if there is empty space, return the index of the space
			if(pEntry->valid==0){
				entry_index =j;
				return entry_index;
			}
	 }
	/* If the set has only 1 entry, return index 0 */
	 // check DEFAULT _CACHE_ASSOC to the number of entries in single set
	 // if it is direct mapped cache, return 0 because it has only 1 entry whose index 0
	 if(DEFAULT_CACHE_ASSOC ==1){
		 return 0;
	 }
	/* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
	// find the oldest access timestamp by check timestamps of all entries
	 else { 
		 for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[cache_index][j];
			if(pEntry->valid==1){
				if(min_timestamp > pEntry->timestamp){
					min_timestamp = pEntry->timestamp;
				}
			}
	    }
		 for (j=0; j <DEFAULT_CACHE_ASSOC; j++){
			 cache_entry_t *pEntry = &cache_array[cache_index][j];
			 if(min_timestamp == pEntry->timestamp){
				 entry_index=j;
			 }
		 }
	 }

	/* return the cache index for copying from memory */
    return entry_index; 
}

/* This function is to return the data in main memory */
int access_memory(void *addr, char type) {
	int block_addr;
	int word_index;   //for finding desired data
	int word_offset;  //for finding desired data
	int mem_index;    //for copying one block from memory
	int set_index;	  //set_index or cache_index
	int entry_index;
	int tag;
	cache_entry_t *pEntry;
	char *pMemory; // char pointer type!

	/* add this main memory access cycle to global access cycle */
	 // update num_access_cycles by adding MEMORY_ACCESS_CYCLE
	num_access_cycles+=MEMORY_ACCESS_CYCLE;

    block_addr= *(int*)addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
	set_index = block_addr % CACHE_SET_SIZE; 
	tag = block_addr / CACHE_SET_SIZE;
	mem_index = block_addr*DEFAULT_CACHE_BLOCK_SIZE_BYTE/WORD_SIZE_BYTE; 
	word_index = *(int *)addr / WORD_SIZE_BYTE ; 
	word_offset = *(int *)addr % WORD_SIZE_BYTE ; 

    /* get the entry index by invoking find_entry_index_in_set() for copying to the cache */
	entry_index=find_entry_index_in_set(set_index);

	/* Fetch the data from the main memory and copy them to the cache */
	pEntry = &cache_array[set_index][entry_index]; 
	 // in this time, memory pointer points at memory_array[mem_index]
	pMemory =(char*)&memory_array[mem_index]; 
	 // copy one block from memory to cache with size DEAULT_CACHE_BLOCK_SIZE_BYTE
	memmove(pEntry->data,pMemory,DEFAULT_CACHE_BLOCK_SIZE_BYTE);

	// update valid bit, tag, timestamp
	pEntry->valid =1;
	pEntry->tag = tag;
	pEntry->timestamp = global_timestamp;

	// in this time, memory pointer points at memory_array[word_index]
	pMemory =(char*)&memory_array[word_index];

	/* Return the accessed data with a suitable type (b, h, or w)*/
	if(type =='b')
		return *(char*)(pMemory+word_offset);
	else if (type =='h')
		return *(short*)(pMemory+word_offset);
	else if (type=='w')
		return *(int*)(pMemory+word_offset);

	// return -1 for unknown type
	return-1;

}
