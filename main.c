/*
 * main.c
 *
 * 20493-02 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 13, 2017
 *
 */

#include <stdio.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void *addr, char data_type) {
    int value_returned = -1; /* accessed data */

	/* Retrieve data from cache by invoking check_cache_data_hit() */
	value_returned = check_cache_data_hit(addr, data_type);

	/* In case of the cache miss event, retreive data from the main memory by invoking access_memory() */
	if(value_returned == -1){
		value_returned = access_memory(addr,data_type);
	}

	/* If there is no data neither in cache nor memory, return -1,else return data */
    return value_returned;    
}

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
	double hit_ratio;
	double bandwidth;

    init_memory_content();
    init_cache_content();
    
    ifp = fopen("access_input2.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("access_output2.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }
    
	fprintf(ofp,"[Accessed data]\n");

	/* read each line and get the data in given (address, type) by invoking retrieve_data() */
	while(1){
		fscanf(ifp,"%d %c",&access_addr,&access_type);
		if(feof(ifp)) break; //End of File
		accessed_data = retrieve_data(&access_addr,access_type); // retrieve data and return its value
		fprintf(ofp,"%d\t%c\t%#x\n",access_addr,access_type,accessed_data);
		global_timestamp++;  //increase global timestamp after one data access trial
	
		// increase number of accessed bytes with a suitable type (b, h, or w)
		if(access_type =='b')
			num_bytes+=1;
		else if (access_type =='h')
			num_bytes+=2;		
		else if (access_type=='w')
			num_bytes+=4;
	}


	/* print hit ratio and bandwidth for each cache mechanism as regards to cache association size */
	hit_ratio = (double)num_cache_hits/(num_cache_hits+num_cache_misses);
	bandwidth =(double)num_bytes/num_access_cycles;
	fprintf(ofp,"-----------------------------------------\n");
	if(DEFAULT_CACHE_ASSOC==1)
		fprintf(ofp,"[Direct mapped cache performance]\n");
	else if(DEFAULT_CACHE_ASSOC==2)
		fprintf(ofp,"[2-way set associative cache performance]\n");
	else if(DEFAULT_CACHE_ASSOC==4)
		fprintf(ofp,"[Fully associative cache performance]\n");
	fprintf(ofp,"Hit ratio = %.2f (%d/%d)\n",hit_ratio,num_cache_hits,num_cache_hits+num_cache_misses);
	fprintf(ofp,"Bandwidth = %.2f (%d/%d)\n",bandwidth,num_bytes,num_access_cycles);

    fclose(ifp);
    fclose(ofp);
    
	 //print final cache entries
    print_cache_entries();
    return 0;
}
