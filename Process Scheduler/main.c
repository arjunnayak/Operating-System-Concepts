//BEST CPU 2 - Process Scheduler
//Arjun Nayak & Ricardo Camarena
//CS 149 Prof. Badari

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define END_MEMORY 50
#define BEGIN_MEMORY 40
#define SIZE_OF_MEMORY 50
#define START_NUM_PROCESSES 6
#define NUM_ITERATIONS 5

enum STATE { EXECUTING, WAITING, READY};
static char* stateEnum[] = {"EXECUTING", "WAITING", "READY"};

typedef struct {
    enum STATE prstate; //The current status of the process (e.g., whether the process is currently executing or waiting)
    int prprio; //The scheduling priority of the process
    int prstkptr; //The saved value of the process’s stack pointer when the process is not executing
    int* prstkbase; //The address of the base of the process’s stack
    //int prstklen; //A limit on the maximum size that the process’s stack can grow
    char prname; //A name assigned to the process that humans use to identify the process’s purpose
    int queued; //boolean identifying if process has been put into the queue
    char instructions[3][6]; //instructions within process
} process;

//instruction methods
void fetch(int memLocation, int* toReg);
void store(int fromReg, int memLocation);
void add(int val1, int val2, int* regToStore);
void subtract(int val1, int val2, int* regToStore);
void multiply(int val1, int val2, int* regToStore);
void divide(int val1, int val2, int* regToStore);
void copyReg(int regToCopy, int* regToStore);
void increment(int* regToIncrement);

//process-related methods
void runNullProcess(int numIterations);
void generateProcessQueue(process ptable[], process queue[], int processes);
int numToReg(int num);
void initializeProcesses(process processTable[]);
process randomizeProcess(process p);
void runProcess(process currentProcess);
void decodeInstruction(char instruction[]);
void displayProcessTable(process processTable[], int numProcesses);
process currentRunningProcess;
process nullProcess = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

//cpu related methods & variables
void printCPUState(void);
void setupRegistersAndMemory(void);
int memory[SIZE_OF_MEMORY];
int eax, ebx, ecx, edx, esi, edi, ebp, esp;
int programCounter = 1;

//int main(int argc, const char * argv[]) {
//    srand((unsigned int)time(NULL));
//    
//    //initial setup
//    setupRegistersAndMemory();
//    int numProcesses = START_NUM_PROCESSES;
//    process processTable[numProcesses];
//    initializeProcesses(processTable);
//    process queue[numProcesses];
//    
//    int i;
//    for (i = 0; i < NUM_ITERATIONS; i = i+1) {
//        printf("run %d \n", i + 1);
//        printf("generating process table and queue with randomized priorities and states \n");
//        generateProcessQueue(processTable, queue, numProcesses); //this randomizes priorities and states and generates ordered queue
//        printf("context switching to highest priority, READY process: process %c \n", queue[0].prname);
//        currentRunningProcess = queue[0]; //sets current running process to highest priority READY process  (context switch and loading to memory)
//        runProcess(currentRunningProcess); //run instructions within process
//        printf("context switching to null process to prepare for printing \n");
//        currentRunningProcess = nullProcess; //after process runs, context switch to null process to prepare for printing
//        runNullProcess(100);
//        displayProcessTable(processTable, numProcesses); //print process table
//        
//        int time = rand() % 5; //generates random number of seconds less than 5 seconds
//        printf("waiting for %d second(s)...\n\n", time);
//        sleep(time);
//    }
//    return 0;
//}

/*
 * 1. Generates processes with randomized states and priorities, and populates process table
 * 2. Orders processes in process table and populates queue (considers only READY processes, not WAITING ones)
 * param: process table references to populate
 * param: queue reference to populate
 * param: number of processes user wants
 */
void generateProcessQueue(process processTable[], process queue[], int numProcesses) {
    int i;
    for (i = 0; i < numProcesses; ++i) { //generates random priorities and states
        process temp = randomizeProcess(processTable[i]);
        processTable[i] = temp;
    }
    
    printf("ordered queue: \n");
    int j;
    int queueCount = 0;
    for(i = 0; i < numProcesses; i = i + 1) {
        process min = { WAITING, 11, 5, 0, 'a'};
        int savedIndex = 0;
        
        for(j = 0; j < numProcesses; j++) {
            if((processTable[j].queued) == 0) {
                if(processTable[j].prstate == READY) {
                    if(processTable[j].prprio < min.prprio) {
                        min = processTable[j];
                        savedIndex = j;
                    } else if(processTable[j].prprio == min.prprio) { //first come first serve if priorities are same
                        ;
                    }
                } else if(processTable[j].prstate == WAITING) { //excludes processes that are "WAITING"
                    processTable[j].queued = -1;
                    //                    printf("process %c waiting \n", processTable[j].prname);
                }
            }
        }
        if(min.prprio < 11) { //if minimum is not default min
            queue[queueCount] = min;
            queue[queueCount].queued = 1;
            processTable[savedIndex].queued = 1;
            printf("    process %c with priority: %d \n", queue[queueCount].prname, queue[queueCount].prprio);
            queueCount = queueCount + 1;
        }
    }
}

/*
 * randomizes process state and priority
 * param: refernece to process
 */
process randomizeProcess(process p) {
    int randStateNum = rand() % 2;
    enum STATE randState;
    switch (randStateNum) {
        case 0:
            randState = READY;
            break;
        case 1:
            randState = WAITING;
            break;
        default:
            printf("not right");
            break;
    }
    int randPriority = rand() % 10;
    process pRet = {randState, randPriority, p.prstkptr, p.prstkbase, p.prname, 0};
    return pRet;
}

/*
 * runs instructions, calls decode instructions (that will check for opcodes and run respective instruction)
 * param: process to run
 */
void runProcess(process currentProcess) {
    int i, j;
    char codedInstruction[6];
    for(i = 0; i < 3; i = i+1)
    {
        for(j = 0; j < 6; j = j+1)
        {
            codedInstruction[j] = currentProcess.instructions[i][j]; //saves instruction in temporary char[] array
        }
        decodeInstruction(codedInstruction);
    }
    printf("process %c done\n", currentProcess.prname);
}

/*
 * displays processes in process table, including namme, state, priority, and stack pointer
 * param: process table reference
 * param: number of processes user wants
 */
void displayProcessTable(process processTable[], int numProcesses) {
    printf("Displaying current process table \n");
    int i;
    for (i = 0; i < numProcesses; i = i + 1) {
        process temp = processTable[i];
        printf("process %c: \n", temp.prname);
        printf("    state: %s \n", stateEnum[temp.prstate]);
        printf("    priority: %d \n", temp.prprio);
        printf("    stack pointer: %d \n", temp.prstkptr);
    }
    printf("\n");
}

/*
 * Runs null process
 * param: number of iterations
 */
void runNullProcess(int numIterations) {
    int i = 0;
    while(i < numIterations) {
        i = i + 1;
    }
}

/*
 * decodes instruction determined by op codes provided
 * param: array of characters representing 1 instruction
 */
void decodeInstruction(char instruction[]) {
    int memVal;
    int regVal;
    int actualReg;
    
    if(instruction[0] == '0' && instruction[1] == '0') { //fetch
        memVal = instruction[2] - '0';
        regVal = instruction[3] - '0';
        actualReg = numToReg(regVal);
        fetch(memVal, &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '1'){ //store
        memVal = instruction[2] - '0';
        regVal = instruction[3] - '0';
        actualReg = numToReg(regVal);
        store(actualReg, memVal);
    }
    else if (instruction[0] == '0' && instruction[1] == '2'){ //add
        regVal = instruction[4] - '0';
        actualReg = numToReg(regVal);
        add((instruction[2] - '0'), (instruction[3] - '0'), &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '3'){ //subtract
        regVal = instruction[4];
        actualReg = numToReg(regVal);
        subtract((instruction[2] - '0'), (instruction[3] - '0'), &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '4'){ //multiply
        regVal = instruction[4] - '0';
        actualReg = numToReg(regVal);
        multiply((instruction[2] - '0'), (instruction[3] - '0'), &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '5'){ //divide
        regVal = instruction[4] - '0';
        actualReg = numToReg(regVal);
        divide((instruction[2] - '0'), (instruction[3] - '0'), &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '6'){ //copyReg
        int tempReg;
        int tempRegVal;
        tempRegVal = instruction[2] - '0';
        tempReg = numToReg(tempRegVal);
        regVal = instruction[3] - '0';
        actualReg = numToReg(regVal);
        copyReg(tempReg, &actualReg);
    }
    else if (instruction[0] == '0' && instruction[1] == '7'){ //increment
        regVal = instruction[2];
        actualReg = numToReg(regVal);
        increment(&actualReg);
    }
}

/* 
 * translates number from instruction character array to respective register
 * param: number representing register
 */
int numToReg(int num){
    switch (num) {
        case 1:
            num = eax;
            break;
        case 2:
            num = ebx;
            break;
        case 3:
            num = ecx;
            break;
        case 4:
            num = edx;
            break;
        case 5:
            num = esi;
            break;
        case 6:
            num = edi;
            break;
        case 7:
            num = ebp;
            break;
        case 8:
            num = esp;
            break;
        default:
            printf("ERROR! ERROR! REGISTERS ARE FROM 1 - 8");
            break;
    }
    return num;
}

/*
 * prints registers, base/stack pointer, program counter, and memory
 */
void printCPUState(void) {
    printf("Begin Printing CPU State...\n");
    printf("Registers: \n");
    printf("EAX: %d \n", eax);
    printf("EBX: %d \n", ebx);
    printf("ECX: %d \n", ecx);
    printf("EDX: %d \n", edx);
    printf("ESI: %d \n", esi);
    printf("EDI: %d \n", edi);
    printf("Base Pointer EBP: %d \n", ebp);
    printf("Stack Pointer ESP: %d \n", esp);
    printf("Program Counter: %d \n", programCounter);
    printf("Memory: \n");
    int i;
    for(i = BEGIN_MEMORY; i < END_MEMORY; i = i + 1) {
        printf("%d: %d ", i, memory[i]);
    }
    printf("\n");
    printf("End Printing CPU State. \n\n");
}

/*
 * initial setup of registers and memory
 */
void setupRegistersAndMemory(void) {
    //Registers
    eax, ebx, ecx, edx, esi, edi, ebp, esp = 0;
    
    //Memory
    int i;
    int count = 0x01;
    
    for(i = BEGIN_MEMORY; i < END_MEMORY; i = i + 1) {
        memory[i] = count;
        count = count * 2;
    }
}

/*
 * initializes processes, character arrays represent instructions per process
 * this is called once, before iterations
 * param: process table reference to populate
 */
void initializeProcesses(process processTable[]) {
    int processNum = 0;
    process p1 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','0','1','2','3','4'}, {'0','0','2','3','4','5'}, {'0','1','1','2','3','4'} } };
    processTable[processNum++] = p1;
    process p2 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','1','2','3','4','5'}, {'0','2','1','2','3','4'}, {'0','2','2','3','4','5'} } };
    processTable[processNum++] = p2;
    process p3 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','3','1','2','3','4'}, {'0','3','2','3','4','5'}, {'0','4','1','2','3','4'} } };
    processTable[processNum++] = p3;
    process p4 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','4','2','3','4','5'}, {'0','5','1','2','3','4'}, {'0','5','2','3','4','5'} } };
    processTable[processNum++] = p4;
    process p5 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','6','1','2','3','4'}, {'0','6','2','3','4','5'}, {'0','7','1','2','3','4'} } };
    processTable[processNum++] = p5;
    process p6 = { READY, 0, processNum * 5, 0, processNum + '0', 0, { {'0','7','2','3','4','5'}, {'0','8','1','2','3','4'}, {'0','8','2','3','4','5'} } };
    processTable[processNum++] = p6;
}


//The rest of the methods represent some pretty straightforward instructions

//void fetch(int memLocation, int* toReg) {
//    *toReg = memory[memLocation];
//    printCPUState();
//    programCounter++;
//}
//
//void store(int fromReg, int memLocation) {
//    memory[memLocation] = fromReg;
//    printCPUState();
//    programCounter++;
//}

void add(int val1, int val2, int* regToStore) {
    *regToStore = val1 + val2;
}

void subtract(int val1, int val2, int* regToStore) {
    *regToStore = val1 - val2;
}

void multiply(int val1, int val2, int* regToStore) {
    *regToStore = val1 * val2;
}

void divide(int val1, int val2, int* regToStore) {
    *regToStore = val1 / val2;
}

void copyReg(int regToCopy, int* regToStore) {
    *regToStore = regToCopy;
}

void increment(int* regToIncrement) {
    *regToIncrement = *regToIncrement + 1;
}