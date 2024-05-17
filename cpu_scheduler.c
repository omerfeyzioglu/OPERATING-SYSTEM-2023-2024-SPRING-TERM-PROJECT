#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define RAM_SIZE 2048
#define PRIORITY_0_RAM 512
#define TIME_QUANTUM_2 8
#define TIME_QUANTUM_3 16

//nnnn
typedef struct {
    char process_number[10];
    int arrival_time;
    int priority;
    int burst_time;
    int ram;
    int cpu_rate;
} Process;

typedef struct Node {
    Process process;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} Queue;

void initQueue(Queue* q) {
    q->front = q->rear = NULL;
}

void enqueue(Queue* q, Process p) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->process = p;
    newNode->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

Process dequeue(Queue* q) {
    if (q->front == NULL) {
        printf("Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    Node* temp = q->front;
    Process p = temp->process;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return p;
}

int isQueueEmpty(Queue* q) {
    return q->front == NULL;
}

Queue priority0Queue;
Queue priority1Queue;
Queue priority2Queue;
Queue priority3Queue;

int totalRam = RAM_SIZE;
int priority0Ram = PRIORITY_0_RAM;
int otherRam = RAM_SIZE - PRIORITY_0_RAM;

FILE *outputFile;

void readProcesses(const char* filename, Process processes[], int* processCount) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    *processCount = 0;
    while (fgets(line, sizeof(line), file)) {
        Process p;
        sscanf(line, "%[^,],%d,%d,%d,%d,%d",
               p.process_number, &p.arrival_time, &p.priority, &p.burst_time, &p.ram, &p.cpu_rate);
        processes[(*processCount)++] = p;
    }
    fclose(file);
}

void scheduleProcesses(Process processes[], int processCount) {
    for (int i = 0; i < processCount; i++) {
        Process p = processes[i];
        if (p.priority == 0) {
            if (p.ram <= priority0Ram) {
                priority0Ram -= p.ram;
                fprintf(outputFile, "Process %s is queued to be assigned to CPU-1.\n", p.process_number);
                enqueue(&priority0Queue, p);
            }
        } else {
            if (p.ram <= otherRam) {
                otherRam -= p.ram;
                switch (p.priority) {
                    case 1:
                        enqueue(&priority1Queue, p);
                        fprintf(outputFile, "Process %s is placed in the que1 queue to be assigned to CPU-2.\n", p.process_number);
                        break;
                    case 2:
                        enqueue(&priority2Queue, p);
                        fprintf(outputFile, "Process %s is placed in the que2 queue to be assigned to CPU-2.\n", p.process_number);
                        break;
                    case 3:
                        enqueue(&priority3Queue, p);
                        fprintf(outputFile, "Process %s is placed in the que3 queue to be assigned to CPU-2.\n", p.process_number);
                        break;
                }
            }
        }
    }
}

void sortQueueByBurstTime(Queue* q) {
    if (isQueueEmpty(q)) return;

    Node* sorted = NULL;
    while (q->front != NULL) {
        Node* temp = q->front;
        q->front = q->front->next;

        if (sorted == NULL || temp->process.burst_time < sorted->process.burst_time) {
            temp->next = sorted;
            sorted = temp;
        } else {
            Node* current = sorted;
            while (current->next != NULL && current->next->process.burst_time < temp->process.burst_time) {
                current = current->next;
            }
            temp->next = current->next;
            current->next = temp;
        }
    }
    q->front = sorted;
    q->rear = NULL;
    Node* current = q->front;
    while (current && current->next) {
        current = current->next;
    }
    q->rear = current;
}

void roundRobinScheduling(Queue* q, int quantum) {
    Queue tempQueue;
    initQueue(&tempQueue);
    while (!isQueueEmpty(q)) {
        Process p = dequeue(q);
        if (p.burst_time > quantum) {
            p.burst_time -= quantum;
            fprintf(outputFile, "Process %s run until the defined quantum time and is queued again because the process is not completed.\n", p.process_number);
            enqueue(&tempQueue, p);
        } else {
            fprintf(outputFile, "Process %s is assigned to CPU-2.\n", p.process_number);
            fprintf(outputFile, "Process %s is completed and terminated.\n", p.process_number);
        }
    }
    while (!isQueueEmpty(&tempQueue)) {
        enqueue(q, dequeue(&tempQueue));
    }
}

void dispatchProcesses() {
    while (!isQueueEmpty(&priority0Queue)) {
        Process p = dequeue(&priority0Queue);
        fprintf(outputFile, "Process %s is assigned to CPU-1.\n", p.process_number);
        fprintf(outputFile, "Process %s is completed and terminated.\n", p.process_number);
    }

    sortQueueByBurstTime(&priority1Queue);
    while (!isQueueEmpty(&priority1Queue)) {
        Process p = dequeue(&priority1Queue);
        fprintf(outputFile, "Process %s is assigned to CPU-2.\n", p.process_number);
        fprintf(outputFile, "Process %s is completed and terminated.\n", p.process_number);
    }

    roundRobinScheduling(&priority2Queue, TIME_QUANTUM_2);
    while (!isQueueEmpty(&priority2Queue)) {
        Process p = dequeue(&priority2Queue);
        fprintf(outputFile, "Process %s is assigned to CPU-2.\n", p.process_number);
        fprintf(outputFile, "Process %s is completed and terminated.\n", p.process_number);
    }

    roundRobinScheduling(&priority3Queue, TIME_QUANTUM_3);
    while (!isQueueEmpty(&priority3Queue)) {
        Process p = dequeue(&priority3Queue);
        fprintf(outputFile, "Process %s is assigned to CPU-2.\n", p.process_number);
        fprintf(outputFile, "Process %s is completed and terminated.\n", p.process_number);
    }
}

void printQueue(Queue* q) {
    Node* current = q->front;
    while (current != NULL) {
        printf("%s", current->process.process_number);
        current = current->next;
        if (current != NULL) printf("-");
    }
    printf("\n");
}

void printQueues() {
    printf("CPU-1 que1(priority-0) (FCFS)→");
    printQueue(&priority0Queue);

    printf("CPU-2 que2(priority-1) (SJF)→");
    printQueue(&priority1Queue);

    printf("CPU-2 que3(priority-2) (RR-q8)→");
    printQueue(&priority2Queue);

    printf("CPU-2 que4(priority-3) (RR-q16)→");
    printQueue(&priority3Queue);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Process processes[MAX_PROCESSES];
    int processCount;

    initQueue(&priority0Queue);
    initQueue(&priority1Queue);
    initQueue(&priority2Queue);
    initQueue(&priority3Queue);

    readProcesses(argv[1], processes, &processCount);

    outputFile = fopen("output.txt", "w");
    if (outputFile == NULL) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    scheduleProcesses(processes, processCount);
    dispatchProcesses();

    fclose(outputFile);

    printQueues();

    return 0;
}
