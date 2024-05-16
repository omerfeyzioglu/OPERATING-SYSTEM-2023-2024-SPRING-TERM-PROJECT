#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define RAM_SIZE 2048
#define HIGH_PRIORITY 1
#define MEDIUM_PRIORITY 2
#define LOW_PRIORITY 3
#define HIGH_PRIORITY_QUANTUM 8
#define MEDIUM_PRIORITY_QUANTUM 8
#define LOW_PRIORITY_QUANTUM 16

// Process structure
typedef struct {
    char process_number[5];
    int arrival_time;
    int priority;
    int burst_time;
    int ram_mb;
    int cpu_rate;
} Process;

// Queue structure
typedef struct Node {
    Process data;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} Queue;

// Function prototypes
Queue* createQueue();
void enqueue(Queue* queue, Process data);
Process dequeue(Queue* queue);
bool isQueueEmpty(Queue* queue);
bool resourceCheck(Process process);
void assignProcessToCPU1(Process process);
void assignProcessToCPU2(Process process);
void executeProcessOnCPU1(FILE* outputFile);
void executeProcessOnCPU2(int queueIndex, FILE* outputFile);
void printQueuedProcesses(FILE* outputFile);

// Global variables
Queue* cpu1Queue;
Queue* cpu2Queues[3]; // 0: high priority, 1: medium priority, 2: low priority

int main(int argc, char* argv[]) {
    // Open input file
    FILE* inputFile = fopen("input.txt", "r");
    FILE* outputFile = fopen("output.txt", "w");

    if (inputFile == NULL || outputFile == NULL) {
        printf("Error opening input/output file.\n");
        return 1;
    }

    // Initialize queues
    cpu1Queue = createQueue();
    for (int i = 0; i < 3; i++) {
        cpu2Queues[i] = createQueue();
    }

    // Read processes from input file and perform scheduling
    char line[100];
    while (fgets(line, sizeof(line), inputFile)) {
        Process process;
        sscanf(line, "%[^,],%d,%d,%d,%d,%d", process.process_number, &process.arrival_time, &process.priority,
               &process.burst_time, &process.ram_mb, &process.cpu_rate);
        
        if (resourceCheck(process)) {
            if (process.priority == 0) {
                assignProcessToCPU1(process);
            } else {
                assignProcessToCPU2(process);
            }
        }
    }

    // Execute processes on CPUs
    executeProcessOnCPU1(outputFile);
    for (int i = 0; i < 3; i++) {
        executeProcessOnCPU2(i, outputFile);
    }

    // Print queued processes after all processes are executed
    printQueuedProcesses(outputFile);

    // Close input and output files
    fclose(inputFile);
    fclose(outputFile);

    return 0;
}

// Create an empty queue
Queue* createQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = queue->rear = NULL;
    return queue;
}

// Enqueue a process to the rear of the queue
void enqueue(Queue* queue, Process data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Dequeue a process from the front of the queue
Process dequeue(Queue* queue) {
    if (queue->front == NULL) {
        Process emptyProcess = {"", 0, 0, 0, 0, 0};
        return emptyProcess;
    }
    Node* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    Process data = temp->data;
    free(temp);
    return data;
}

// Check if the queue is empty
bool isQueueEmpty(Queue* queue) {
    return queue->front == NULL;
}

// Perform resource check
bool resourceCheck(Process process) {
    if (process.priority == 0) {
        return process.ram_mb <= 512;
    } else {
        return process.ram_mb <= RAM_SIZE - 512;
    }
}

// Assign process to CPU-1
// Assign process to CPU-1
void assignProcessToCPU1(Process process) {
    printf("CPU-1 que1(priority-0) (FCFS)→%s-\n", process.process_number);
    enqueue(cpu1Queue, process);
}

// Assign process to CPU-2
void assignProcessToCPU2(Process process) {
    printf("CPU-2 que%d(priority-%d) (", process.priority + 2, process.priority + 1);
    if (process.priority == 0) {
        printf("SJF)→%s-", process.process_number);
    } else {
        printf("RR-q%d)→%s-", (process.priority == 1) ? HIGH_PRIORITY_QUANTUM : LOW_PRIORITY_QUANTUM, process.process_number);
    }
    enqueue(cpu2Queues[process.priority], process);
    printf("\n");
}



// Execute process on CPU-1 (FCFS)
void executeProcessOnCPU1(FILE* outputFile) {
    while (!isQueueEmpty(cpu1Queue)) {
        Process process = dequeue(cpu1Queue);
        fprintf(outputFile, "Process %s is queued to be assigned to CPU-1.\n", process.process_number);
        fprintf(outputFile, "Process %s is assigned to CPU-1.\n", process.process_number);
        fprintf(outputFile, "Process %s is completed and terminated.\n\n", process.process_number);
    }
}

// Execute process on CPU-2
void executeProcessOnCPU2(int queueIndex, FILE* outputFile) {
    while (!isQueueEmpty(cpu2Queues[queueIndex])) {
        Process process = dequeue(cpu2Queues[queueIndex]);
        fprintf(outputFile, "Process %s is placed in the que%d queue to be assigned to CPU-2.\n", process.process_number, queueIndex + 2);
        fprintf(outputFile, "Process %s is assigned to CPU-2.\n", process.process_number);
        if (process.burst_time <= ((queueIndex == 0) ? HIGH_PRIORITY_QUANTUM : LOW_PRIORITY_QUANTUM)) {
            fprintf(outputFile, "The operation of process %s is completed and terminated.\n\n", process.process_number);
        } else {
            fprintf(outputFile, "Process %s run until the defined quantum time and is queued again because the process is not completed.\n", process.process_number);
            process.burst_time -= ((queueIndex == 0) ? HIGH_PRIORITY_QUANTUM : LOW_PRIORITY_QUANTUM);
            enqueue(cpu2Queues[queueIndex], process);
            fprintf(outputFile, "Process %s is assigned to CPU-2, its operation is completed and terminated.\n\n", process.process_number);
        }
    }
}

// Print queued processes
// Print queued processes
void printQueuedProcesses(FILE* outputFile) {
    fprintf(outputFile, "CPU-1 que1(priority-0) (FCFS)→");
    Queue* currentQueue = cpu1Queue;
    while (!isQueueEmpty(currentQueue)) {
        Process process = dequeue(currentQueue);
        fprintf(outputFile, "%s-", process.process_number);
        enqueue(currentQueue, process); // Add the process back to the queue
    }
    fprintf(outputFile, "\n");

    // Print CPU-2 queues
    for (int i = 0; i < 3; i++) {
        fprintf(outputFile, "CPU-2 que%d(priority-%d) (", i + 2, i + 1);
        if (i == 0) {
            fprintf(outputFile, "SJF)→");
        } else {
            fprintf(outputFile, "RR-q%d)→", (i == 1) ? HIGH_PRIORITY_QUANTUM : LOW_PRIORITY_QUANTUM);
        }
        currentQueue = cpu2Queues[i];
        while (!isQueueEmpty(currentQueue)) {
            Process process = dequeue(currentQueue);
            fprintf(outputFile, "%s-", process.process_number);
            enqueue(currentQueue, process); // Add the process back to the queue
        }
        fprintf(outputFile, "\n");
    }
}

