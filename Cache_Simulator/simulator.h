/* Functions for parsing an input memory trace
 * Inspired and adapted from CS 3410 @ Cornell
 * Lillian Pentecost, 2022
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// Define structs related to collecting and
// reporting results

typedef struct results {
    int total_accesses;
    int reads;
    int writes;
    int read_hits;
    int write_hits;
    int read_misses;
    int write_misses;
} results_s;

// Define struct representing any 1 action
typedef struct action {
    uint32_t addr;
    bool read;
} action_s;

// Given an input file with a simple format, 
// parse into an array of actions
void parseTrace(FILE* tf);

// print results
void printResults();

