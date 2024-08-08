/* Functions for parsing an input memory trace
 * and running a basic cache simulation
 * Inspired and adapted from CS 3410 @ Cornell
 * Lillian Pentecost, 2022
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "simulator.h"
#include "cache.c"

#define MAX_ACCESS_COUNT 100 // accesses

// Global results variable
results_s results;

action_s access_list[MAX_ACCESS_COUNT];


void printResults(){
    printf("\t**Summary of Cache Simulation Results**\n");
    printf("\t\tTotal Accesses: \t%d\n", results.total_accesses);
    printf("\t\tTotal Reads: \t%d\n", results.reads);
    printf("\t\tTotal Writes: \t%d\n", results.writes);
    printf("\t\tRead Cache Hits: \t%d\n", results.read_hits);
    printf("\t\tRead Cache Misses: \t%d\n", results.read_misses);
    printf("\t\tWrite Cache Hits: \t%d\n", results.write_hits);
    printf("\t\tWrite Cache Misses: \t%d\n", results.write_misses);
}

void printTrace(){
    int i = 0;
    printf("\tSummary of Access List from Trace Input: \n");
    while (i < MAX_ACCESS_COUNT){
        printf("\t\t%d : {Address, %x}, {Read, %d}\n", i, access_list[i].addr, access_list[i].read);  
        i++;
    }
}


void parseTrace(FILE* tf){
    // this parsing function relies on a simple and consistent format in the
    // input tracing file; each line should have 1 char representing whether 
    // the access is a read or a write (1 for read, 0 for write), 1 space, and
    // then 8 characters representing the address of the access in hex format
    // For examples, see provided traces
    
    // Go line by line in the file, initialize and assign access-by-access, update results, return accesses
    results.total_accesses = 0;
    results.reads = 0;
    results.writes = 0;

    int length = 12;
    char line[length]; // assign to max length per line
    bool read_write;

    while (fgets(line, length, tf)) {
        // split into read/write and address
        read_write = (bool) atoi(line);
        // assign read/write
        access_list[results.total_accesses].read = read_write;
        // assign address        
        access_list[results.total_accesses].addr = strtoul(&line[2], NULL, 16);

        results.total_accesses++;
        if (read_write) {
            results.reads++;
        } else {
            results.writes++;
        }
    } // stop reading when we reach end of the file

    // optional; print results / trace after parsing to debug
    //printResults();
    //printTrace();
}

int main(int argc, char* argv[]){
    // simulator will take 2 command line arguments
    // argv[1] file name of cache config
    // argv[2] file name of example trace of memory accesses 
    
    printf("\n ~~ COSC 171 Cache Simulator ~~\n");

    if (argc < 3){
        printf("Incorrect number of command line arguments; please provide a file name for the cache design configuration and the memory trace.\n");
        return -1;
    }

    // parse the cache config, initialize cache
    char* config_file_name = argv[1];
    printf("\n\tFile name for configuration is: %s\n", config_file_name);
    FILE* config_file;

    // open a file
    config_file = fopen(config_file_name, "r");
    
    // initialize cache design parameters
    uint32_t blockSize = 0;
    uint32_t blocksPerSet = 0;
    uint32_t numSets = 0;

    // test if file opened successfully; exit if it failed
    if (config_file == NULL) {
        printf("Could not open configuration file %s\n", config_file_name);
        return -1; 
    } else {
        printf("\tParsing Configuration file.\n");
        fscanf(config_file, "Block Size: %u\n", &blockSize);
        fscanf(config_file, "Blocks Per Set (1 for DM): %u\n", &blocksPerSet);
        fscanf(config_file, "Number of Sets: %u\n", &numSets);
    }
    fclose(config_file); // close a file when done with parsing
    
    // initialize cache using config
    printf("\tParsed Configuration; Initializing Cache.\n");
    cache_init(blockSize, numSets, blocksPerSet);

    // next, parse input trace, set reads/writes/accesses 
    // parseTrace returns array of actions
    char* memory_trace_file_name = argv[2];
    printf("\n\tFile name for memory address trace is: %s\n", memory_trace_file_name);
    FILE* trace_file;

    // open a file
    trace_file = fopen(memory_trace_file_name, "r");
    // test if file opened successfully; exit if it failed
    if (config_file == NULL) {
        printf("Could not open memory trace file %s\n", memory_trace_file_name);
        return -1; 
    } else {
        printf("\tParsing Memory Trace file.\n");
        parseTrace(trace_file);
    }
    fclose(trace_file); // close a file when done with parsing


    // now, we simulate action-by-action to record hits + misses
    // print lots of details along the way
    for (int i=0; i<results.total_accesses; i++){
        bool is_a_hit = cache_access(access_list[i].addr, access_list[i].read);
        if (is_a_hit && access_list[i].read){
            results.read_hits++;
        } 
        if (is_a_hit && !(access_list[i].read)){
            results.write_hits++;
        }
        if(!is_a_hit && access_list[i].read){
            results.read_misses++;
        } 
        if(!is_a_hit && !access_list[i].read) {
            results.write_misses++;
        }
    }

    // print summary of results
    printResults();
}
