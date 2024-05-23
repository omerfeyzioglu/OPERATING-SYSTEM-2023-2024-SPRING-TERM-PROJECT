#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MAX_LINE_LENGTH 256

typedef struct
{
    char process_number[10];
    int arrival_time;
    int priority;
    int burst_time;
    int ram;
    int cpu;
} Process;

typedef struct Node
{
    Process process;
    struct Node *next;
} Node;

typedef struct
{
    Node *front;
    Node *rear;
} Queue;

Queue priority0Queue;
Queue priority1Queue;
Queue priority2Queue;
Queue priority3Queue;

int priority0Ram = 512;
int otherRam = 2048 - 512;
int totalRam = 2048;

FILE *outputFile;

void initQueue(Queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}

void enqueue(Queue *q, Process p)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->process = p;
    newNode->next = NULL;
    if (q->rear == NULL)
    {
        q->front = newNode;
        q->rear = newNode;
    }
    else
    {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

Process dequeue(Queue *q)
{
    if (q->front == NULL)
    {
        Process emptyProcess = {"", -1, -1, -1, -1, -1};
        return emptyProcess;
    }
    else
    {
        Node *temp = q->front;
        Process p = temp->process;
        q->front = q->front->next;
        if (q->front == NULL)
        {
            q->rear = NULL;
        }
        free(temp);
        return p;
    }
}

void logProcess(const char *message, Process p)
{
    fprintf(outputFile, message, p.process_number);
}

void readProcesses(const char *filename, Process *processes, int *processCount)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    *processCount = 0;

    while (fgets(line, sizeof(line), file))
    {
        sscanf(line, "%[^,],%d,%d,%d,%d,%d",
               processes[*processCount].process_number,
               &processes[*processCount].arrival_time,
               &processes[*processCount].priority,
               &processes[*processCount].burst_time,
               &processes[*processCount].ram,
               &processes[*processCount].cpu);
        (*processCount)++;
    }

    fclose(file);
}

void printQueue(Queue *q)
{
    Node *current = q->front;
    while (current != NULL)
    {
        printf("P%s", current->process.process_number);
        current = current->next;
        if (current != NULL)
            printf("-");
    }
    printf("...");
}

void printQueues()
{
    printf("CPU-1 que1(priority-0) (FCFS)→");
    printQueue(&priority0Queue);
    printf("\n");

    printf("CPU-2 que2(priority-1) (SJF)→");
    printQueue(&priority1Queue);
    printf("\n");

    printf("CPU-2 que3(priority-2) (RR-q8)→");
    printQueue(&priority2Queue);
    printf("\n");

    printf("CPU-2 que4(priority-3) (RR-q16)→");
    printQueue(&priority3Queue);
    printf("\n");
}

void sortQueueByBurstTime(Queue *q)
{
    if (q->front == NULL || q->front->next == NULL)
    {
        return;
    }

    for (Node *i = q->front; i != NULL; i = i->next)
    {
        for (Node *j = i->next; j != NULL; j = j->next)
        {
            if (i->process.burst_time > j->process.burst_time)
            {
                Process temp = i->process;
                i->process = j->process;
                j->process = temp;
            }
        }
    }
}

void processQueues()
{
    Process p;

    // Process priority 0 queue (FCFS)
    while ((p = dequeue(&priority0Queue)).arrival_time != -1)
    {
        logProcess("Process %s is assigned to CPU-1.\n", p);
        logProcess("Process %s is completed and terminated.\n", p);
    }

    // Process priority 1 queue (SJF)
    sortQueueByBurstTime(&priority1Queue);
    while ((p = dequeue(&priority1Queue)).arrival_time != -1)
    {
        logProcess("Process %s is assigned to CPU-2.\n", p);
        logProcess("Process %s is completed and terminated.\n", p);
    }

    // Process priority 2 queue (RR-q8)
    int quantum = 8;
    while ((p = dequeue(&priority2Queue)).arrival_time != -1)
    {
        if (p.burst_time > quantum)
        {
            p.burst_time -= quantum;
            logProcess("Process %s is assigned to CPU-2.\n", p);
            logProcess("Process %s run until the defined quantum time and is queued again because the process is not completed.\n", p);
            enqueue(&priority2Queue, p);
        }
        else
        {
            logProcess("Process %s is assigned to CPU-2.\n", p);
            logProcess("Process %s is completed and terminated.\n", p);
        }
    }

    // Process priority 3 queue (RR-q16)
    quantum = 16;
    while ((p = dequeue(&priority3Queue)).arrival_time != -1)
    {
        if (p.burst_time > quantum)
        {
            p.burst_time -= quantum;
            logProcess("Process %s is assigned to CPU-2.\n", p);
            logProcess("Process %s run until the defined quantum time and is queued again because the process is not completed.\n", p);
            enqueue(&priority3Queue, p);
        }
        else
        {
            logProcess("Process %s is assigned to CPU-2.\n", p);
            logProcess("Process %s is completed and terminated.\n", p);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    Process processes[MAX_PROCESSES];
    int processCount;

    initQueue(&priority0Queue);
    initQueue(&priority1Queue);
    initQueue(&priority2Queue);
    initQueue(&priority3Queue);

    outputFile = fopen("output.txt", "w");
    if (outputFile == NULL)
    {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    readProcesses(argv[1], processes, &processCount);

    for (int i = 0; i < processCount; i++)
    {
        Process p = processes[i];
        if (p.priority == 0)
        {
            if (p.ram > priority0Ram)
            {
                logProcess("Process %s can't be allocated to CPU-1. Insufficient RAM.\n", p);
                continue;
            }
            priority0Ram -= p.ram;
            totalRam -= p.ram;
            enqueue(&priority0Queue, p);
            logProcess("Process %s is queued to be assigned to CPU-1.\n", p);
        }
        else
        {
            if (p.ram > otherRam)
            {
                logProcess("Process %s can't be allocated to CPU-2. Insufficient RAM.\n", p);
                continue;
            }
            otherRam -= p.ram;
            totalRam -= p.ram;
            switch (p.priority)
            {
            case 1:
                enqueue(&priority1Queue, p);
                logProcess("Process %s is placed in the que1 queue to be assigned to CPU-2.\n", p);
                break;
            case 2:
                enqueue(&priority2Queue, p);
                logProcess("Process %s is placed in the que2 queue to be assigned to CPU-2.\n", p);
                break;
            case 3:
                enqueue(&priority3Queue, p);
                logProcess("Process %s is placed in the que3 queue to be assigned to CPU-2.\n", p);
                break;
            default:
                printf("Invalid priority for process %s.\n", p.process_number);
                break;
            }
        }
    }

    processQueues();

    // Print the final queue states to the console
    printQueues();

    fclose(outputFile);

    return 0;
}
