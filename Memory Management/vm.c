
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#define SIZE_OF_MEM 40
#define SIZE_OF_PAGES 4
#define NUM_PAGES 4

typedef struct{
    long words[SIZE_OF_PAGES];
} Page;

long mem[SIZE_OF_MEM]; //10-13 would be virtual memory: aka page files
Page pageTable[NUM_PAGES];
long regA, regB;

void initializeMemory();
void setUpPages();
void fetch(int address, long reg);
void store(int address, long reg);
void printPages();
void printMemory();
void printReg();
void runOperation();

int main() {
    initializeMemory(); //
    setUpPages();
    
    int numRuns = 5;
    int i;
    for(i = 0; i < numRuns; i++) {
        printf("run %d \n", i);
        printf("Memory: \n");
        printPages();
        runOperation();
        printReg();
        printf("\n");
    }
    
    
}

void initializeMemory() {
    srand((unsigned int)time(NULL));
    int i, page_counter;
    page_counter = 0;
    for(i = 0; i < SIZE_OF_MEM; i++) {
        if(i < 10) {
            mem[i] = 0; // reserved for OS MEMORY
        }
        else if((i > 9) && (i < 14)) { //virtual memory
            mem[i] = page_counter;
            page_counter++;
        }
        else {
            mem[i] = rand(); //Physical Memory randomized
        }
    }
    
}

void setUpPages(){
    srand((unsigned int)time(NULL));
    int i;
    int j;
    for(i = 0; i < NUM_PAGES; i++) {
        Page p;
        for(j = 0; j < SIZE_OF_PAGES; j++) {
            p.words[j] = rand(); //creates random longs for page
        }
        pageTable[i] = p;
    }
}


void fetch(int address, long reg){
    int offSet;
    offSet = rand() % 4;
    regA = pageTable[mem[address]].words[offSet];
}

void store(int address, long reg){
    int offSet;
    regB = reg;
    offSet = rand() % 4;
    pageTable[mem[address]].words[offSet] = regB;
}

void printPages() {
    int i;
    for(i = 0; i < NUM_PAGES; i++) {
        int j;
        printf("page %d \n", i);
        Page temp = pageTable[i];
        for(j = 0; j < SIZE_OF_PAGES; j++) {
            printf("%ld \n", temp.words[j]);
        }
    }
}

void runOperation() {
    int op = rand() % 2;
    if(op == 0) {
        int pageNum = rand() % NUM_PAGES;
        int offSet = rand() % SIZE_OF_PAGES;
        printf("Operation: Fetch regA, page number %d offset %d \n", pageNum, offSet);
        fetch(pageNum + 10, regA);
    } else {
        int pageNum = rand() % NUM_PAGES;
        int offSet = rand() % SIZE_OF_PAGES;
        printf("Operation: Store page number %d offset %d regB \n", pageNum, offSet);
        store(pageNum + 10, regB);
    }
}

void printMemory() {
    int i;
    for(i = 10; i < SIZE_OF_MEM; i++) {
        printf("%d: %ld \n", i, mem[i]);
    }
}

void printReg() {
    printf("regA: %ld\n", regA);
    printf("regB: %ld\n", regB);
}