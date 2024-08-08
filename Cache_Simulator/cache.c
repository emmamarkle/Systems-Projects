/* A simple definition for a set-associative cache; 
 * Inspired and adapted from CS 3410 @ Cornell
 * and using a simple cache simulator interface
 * from UM EECS 370 Fall 2022 (see Project 5 
 * instructions for links)
 * Lillian Pentecost, 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_CACHE_SIZE 256  // blocks
#define MAX_BLOCK_SIZE 256 // bytes
#define MIN_BLOCK_SIZE 4 // bytes

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE]; // not used in base version of project
    bool dirty;
    bool valid;
    u_int32_t lruLabel;
    u_int32_t set;
    u_int32_t tag;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    u_int32_t blockSize;
    u_int32_t numSets;
    u_int32_t blocksPerSet;
    u_int32_t offsetBits;
    u_int32_t indexBits;
    u_int32_t tagBits;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(u_int32_t, u_int32_t, enum actionType);
void printCache();

/*
 * Set up the cache with given command line parameters. This is 
 * called once in main(). You must implement this function.
 */
void cache_init(u_int32_t blockSize, u_int32_t numSets, u_int32_t blocksPerSet){
    // COMPLETE ME; given block set, number of sets, and blocks per set, initialize the cache
    // NOTE: for a direct mapped cache, blocksPerSet is 1, so numSets represents number of blocks

    // For the purposes of this project, we are ***not actually tracking/updating the data***
    // Your `cache_access` should do the lookup and keep all metadata up-to-date action-by-action
    // Note, however that you will need to either initialize `data` for each block or modify `printCache`

    //optionally, you might want to use the provided `print cache` to examine cache design / contents 
    //printCache();
 
    	u_int32_t numBlocks = numSets * blocksPerSet;
	// check if cache params are within valid range
	if(numBlocks > MAX_CACHE_SIZE || blockSize < MIN_BLOCK_SIZE || blockSize > MAX_BLOCK_SIZE) {
		printf("Invalid cache design");
		exit(0);
	}
	// Initialize cache struct members
	cache.blockSize = blockSize;
	cache.numSets = numSets;
	cache.blocksPerSet = blocksPerSet;
	cache.offsetBits = log2(cache.blockSize);
	cache.indexBits = log2(cache.numSets);
	cache.tagBits = 32 - cache.offsetBits - cache.indexBits;
	int temp;
	// Initialize cache blocks and set metadata
	for(int i = 0; i < cache.numSets*cache.blocksPerSet; i++){
		temp = i/cache.blocksPerSet;
		cache.blocks[i].set = temp;
	}
	// Print cache configuration for debugging
	printCache();
     
    return;
}


/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should return whether the access is a hit or miss
 * addr is the full address for lookup in the cache (32b)
 * read is `true` for a read and `false` for a write
 * assume each access is 4B
 */
bool cache_access(u_int32_t addr, bool read) {
    // COMPLETE ME; given an address and whether the access is a 
    // read or write, this function should execute the lookup (and
    // any updates, as applicable, report each interaction using 
    // printAction, and return `true` if the result is a hit, 
    // `false` for a miss
    // See instructions for more details.
    

    // check if cache params are within valid range
    	enum actionType type;
    	bool toRet = false;
	// Extract address components
	u_int32_t modAddr = addr;
	u_int32_t offsetAnd = (pow(2,cache.offsetBits)-1);
	u_int32_t offset = addr & offsetAnd;
	modAddr = modAddr >> cache.offsetBits;
	u_int32_t test = pow(2, cache.indexBits)-1;
	u_int32_t index = modAddr & test;
	printf("no off %u and val: %u\n",modAddr, test); // debugging statement
	u_int32_t tag = modAddr >> cache.indexBits;
	printf("Addr: %u\nTag: %u\nIndex: %u\nBlockTag: %u\nRead? (1 is true): %d\n", addr, tag, index, cache.blocks[index].tag, read); // more debugging
	// check if cache params are within valid range
	u_int32_t setIndex = index * cache.blocksPerSet;
	u_int32_t currIndex = setIndex;
	u_int32_t nextSetIndex = (setIndex+cache.blocksPerSet);
	printf("Index: %u, NextSetIndex: %u\n", index, nextSetIndex); // debugging again
	
	while(!toRet && currIndex < nextSetIndex) { 
		if(tag == cache.blocks[currIndex].tag && cache.blocks[currIndex].valid) {
			printf("Tag: %u, Cache block tag: %u\n", tag, cache.blocks[currIndex].tag);
			toRet = true; // check if cache params are within valid range

		}
		else { 
			currIndex=currIndex+1; // check if cache params are within valid range

		}
	
	}
	if(!toRet) {
		// eviction logic for cache miss
		// update cache metadata and print evitcion action
		currIndex = currIndex - 1;
		u_int32_t victimIndex;
		u_int32_t maxLabel = 0;
		bool invalFound = false;
		// iterate thru the blocks in the set to find the victim block
		for(int i = setIndex; i < nextSetIndex; i++) { 
			cache.blocks[i].lruLabel = cache.blocks[i].lruLabel + 1;
			// if if block has higher LRU set victim to higher LRU block
			if(cache.blocks[i].lruLabel > maxLabel&& !invalFound) {
				victimIndex = i;
				maxLabel = cache.blocks[i].lruLabel;
			}
			// if invalid mark as victim block
			if(!cache.blocks[i].valid) {
				victimIndex = i;
				invalFound = true;
				maxLabel = cache.blocks[i].lruLabel;
			}
			printf("Block index: %u, its Lru: %u\n", i, cache.blocks[i].lruLabel);			
		}

		u_int32_t victimAddr = cache.blocks[victimIndex].tag << cache.indexBits;
		// calc victim address by using TIO
		victimAddr = victimAddr + index;
		victimAddr = victimAddr << cache.offsetBits;
		// determine if block is dirty and needs to be written back
		if(cache.blocks[victimIndex].dirty) {
			printAction(victimAddr,cache.blockSize,cacheToMemory);
		}
		else { 
			printAction(victimAddr,cache.blockSize,cacheToNowhere);
		}
		// update metadata for the victim block and prepare for new data
		printf("Evict index: %u, and its Lru: %u\n", victimIndex,cache.blocks[victimIndex].lruLabel);
		cache.blocks[victimIndex].valid = true;
		cache.blocks[victimIndex].dirty = false;
		cache.blocks[victimIndex].tag = tag; // update tag
		currIndex = victimIndex; // update victim index

	}

	else {
		// update lru labels for each cache hit	
		for(int j = setIndex; j < nextSetIndex; j++) {
			cache.blocks[j].lruLabel = cache.blocks[j].lruLabel + 1;	
		}
	}
	// print cache access action
	cache.blocks[currIndex].lruLabel = 0;
	if(!read) {
		cache.blocks[currIndex].dirty = true; // set dirty bit to true
		type = processorToCache;	
	}
	else {
		type = cacheToProcessor;
	}
	printAction((addr-offset), cache.blockSize, type);
    return toRet;
}


/*
 * print any end of run statistics you collected. 
 * This is not required, but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 */
void printStats(){
    return;
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(u_int32_t address, u_int32_t size, enum actionType type)
{
    printf("$$$ transferring address range [0x%x-0x%x] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and you may modify it to your needs
 */
void printCache()
{
    printf("\nCache Design:\n");
    printf("\t Number of Sets:\t%u\n", cache.numSets);
    printf("\t Blocks per Set (1 for DM):\t%u\n", cache.blocksPerSet);
    printf("\t Block Size (B):\t%u\n", cache.blockSize);
    printf("\t Offset Bits (b):\t%u\n", cache.offsetBits);
    printf("\t Index Bits (b):\t%u\n", cache.indexBits);
    printf("\t Tag Bits (b):\t%u\n", cache.tagBits);

    printf("\nCache Contents:\n");
    for (u_int32_t set = 0; set < cache.numSets; ++set) {
        printf("\tset %u:\n", set);
        for (u_int32_t block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %u ]: {", block);
            // NOTE: not tracking / updating actual data blocks, so skip this; print metadata
            //for (int index = 0; index < cache.blockSize; ++index) {
            //    printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            //}
            printf(" }\n");
            printf(" \t valid bit(s) %u\n", cache.blocks[set * cache.blocksPerSet + block].valid); 
            printf(" \t dirty bit(s) %u\n", cache.blocks[set * cache.blocksPerSet + block].dirty); 
            printf(" \t LRU label %u \n", cache.blocks[set * cache.blocksPerSet + block].lruLabel); 
            printf(" \t tag %u \n", cache.blocks[set * cache.blocksPerSet + block].tag); 
            printf(" \t set %u \n", cache.blocks[set * cache.blocksPerSet + block].set); 
        }
    }
    printf("(end cache contents)\n");
}
