#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Constants for system parameters
#define RESERVE_MEM_FOR_PRIORITY_ZERO 512
#define QUEUE_CAP 50
#define MEM_SIZE 2048
#define RR_QUANTUM_LOW 16
#define PRIORITY_HIGH 1
#define PRIORITY_MED 2
#define PRIORITY_LOW 3
#define PRIORITY_ZERO 0
#define RR_QUANTUM_MED 8
#define MAX_PROCS 100

// Structure to represent a process
typedef struct {
    char id[5];
    int ram;
    int priority;
    int arrival;
    int burst;
    int cpu;
} Proc;

// Structure to represent a queue of processes
typedef struct {
    Proc *elements[QUEUE_CAP];
    int head, tail;
} Queue;

// Function prototypes
void schedule_sjf(Proc *procs, int count, FILE *out);
void schedule_fcfs(Proc *procs, int count, FILE *out);
void schedule_rr(Proc *procs, int count, int quantum, int priority, FILE *out);
void enqueue(Queue *queue, Proc *proc);
Proc *dequeue(Queue *queue);
int is_queue_empty(Queue *queue);
int check_resources(int current_ram_usage, int ram_required, int cpu_usage, int cpu_required);

// Main function
int main(int argc, char *argv[]) {
    // Check for correct command line argument
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Open input and output files
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen("output.txt", "w");

    // Check if files were opened successfully
    if (input == NULL || output == NULL) {
        printf("Error opening files!\n");
        return 1;
    }

    // Read process information from input file
    Proc procs[MAX_PROCS];
    int proc_count = 0;
    while (fscanf(input, "%[^,],%d,%d,%d,%d,%d\n", procs[proc_count].id,
                  &procs[proc_count].arrival, &procs[proc_count].priority,
                  &procs[proc_count].burst, &procs[proc_count].ram,
                  &procs[proc_count].cpu) == 6) {
        proc_count++;
    }

    // Close input file
    fclose(input);

    // Schedule processes using different algorithms
    schedule_fcfs(procs, proc_count, output);
    schedule_sjf(procs, proc_count, output);
    schedule_rr(procs, proc_count, RR_QUANTUM_MED, PRIORITY_MED, output);
    schedule_rr(procs, proc_count, RR_QUANTUM_LOW, PRIORITY_LOW, output);

    // Close output file
    fclose(output);

    // Print CPU queues for each priority level
    printf("CPU-1 queue (priority-0, FCFS) -> ");
    for (int i = 0; i < proc_count; i++) {
        if (procs[i].priority == PRIORITY_ZERO) {
            printf("%s ", procs[i].id);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-1, SJF) -> ");
    for (int i = 0; i < proc_count; i++) {
        if (procs[i].priority == PRIORITY_HIGH) {
            printf("%s ", procs[i].id);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-2, RR, q=8) -> ");
    for (int i = 0; i < proc_count; i++) {
        if (procs[i].priority == PRIORITY_MED) {
            printf("%s ", procs[i].id);
        }
    }
    printf("\n");

    printf("CPU-2 queue (priority-3, RR, q=16) -> ");
    for (int i = 0; i < proc_count; i++) {
        if (procs[i].priority == PRIORITY_LOW) {
            printf("%s ", procs[i].id);
        }
    }
    printf("\n");

    return 0;
}

// Schedule processes using First-Come, First-Served algorithm
void schedule_fcfs(Proc *procs, int count, FILE *out) {
    Queue cpu1_queue = { .head = 0, .tail = -1 }; // Initialize queue for CPU-1
    int time = 0;                                 // Initialize time tracker
    int current_ram_usage = RESERVE_MEM_FOR_PRIORITY_ZERO; // Initialize RAM usage

    // Enqueue processes with priority zero to CPU-1 queue
    for (int i = 0; i < count; i++) {
        if (procs[i].priority == PRIORITY_ZERO && check_resources(current_ram_usage, procs[i].ram, procs[i].cpu, procs[i].burst)) {
            current_ram_usage += procs[i].ram;
            fprintf(out, "Process %s is queued to be assigned to CPU-1.\n", procs[i].id);
            enqueue(&cpu1_queue, &procs[i]);
        }
    }

    // Process queued tasks in FCFS order
    while (!is_queue_empty(&cpu1_queue)) {
        Proc *current = dequeue(&cpu1_queue);
        fprintf(out, "Process %s is assigned to CPU-1.\n", current->id);
        time += current->burst;
        fprintf(out, "Process %s is completed and terminated.\n", current->id);
    }
}

// Schedule processes using Shortest Job First algorithm
void schedule_sjf(Proc *procs, int count, FILE *out) {
    Queue cpu2_queue = { .head = 0, .tail = -1 }; // Initialize queue for CPU-2
    int time = 0;                                 // Initialize time tracker
    int current_ram_usage = 0;                    // Initialize RAM usage

    // Enqueue processes with high priority to CPU-2 queue
    for (int i = 0; i < count; i++) {
        if (procs[i].priority == PRIORITY_HIGH && check_resources(current_ram_usage, procs[i].ram, procs[i].cpu, procs[i].burst)) {
            current_ram_usage += procs[i].ram;
            fprintf(out, "Process %s is placed in the queue to be assigned to CPU-2.\n", procs[i].id);
            enqueue(&cpu2_queue, &procs[i]);
        }
    }

    // Process queued tasks in SJF order
    while (!is_queue_empty(&cpu2_queue)) {
        int shortest_idx = -1;
        int shortest_burst = INT_MAX;
        for (int i = cpu2_queue.head; i <= cpu2_queue.tail; i++) {
            if (cpu2_queue.elements[i]->burst < shortest_burst) {
                shortest_burst = cpu2_queue.elements[i]->burst;
                shortest_idx = i;
            }
        }

               if (shortest_idx != -1) {
            Proc *current = cpu2_queue.elements[shortest_idx];
            fprintf(out, "Process %s is assigned to CPU-2.\n", current->id);
            time += current->burst;
            fprintf(out, "The operation of process %s is completed and terminated.\n", current->id);
            
            // Remove completed process from queue
            for (int i = shortest_idx; i < cpu2_queue.tail; i++) {
                cpu2_queue.elements[i] = cpu2_queue.elements[i + 1];
            }
            cpu2_queue.tail--;
        }
    }
}

// Schedule processes using Round Robin algorithm
void schedule_rr(Proc *procs, int count, int quantum, int priority, FILE *out) {
    Queue cpu2_queue = { .head = 0, .tail = -1 }; // Initialize queue for CPU-2
    int time = 0;                                 // Initialize time tracker
    int current_ram_usage = 0;                    // Initialize RAM usage

    // Enqueue processes with specified priority to CPU-2 queue
    for (int i = 0; i < count; i++) {
        if (procs[i].priority == priority && check_resources(current_ram_usage, procs[i].ram, procs[i].cpu, procs[i].burst)) {
            current_ram_usage += procs[i].ram;
            fprintf(out, "Process %s is placed in the queue to be assigned to CPU-2.\n", procs[i].id);
            enqueue(&cpu2_queue, &procs[i]);
        }
    }

    // Process queued tasks using Round Robin scheduling
    while (!is_queue_empty(&cpu2_queue)) {
        Proc *current = dequeue(&cpu2_queue);
        fprintf(out, "Process %s is assigned to CPU-2.\n", current->id);

        if (current->burst > quantum) {
            // Process for quantum time and re-enqueue if not completed
            time += quantum;
            current->burst -= quantum;
            fprintf(out, "Process %s ran for the defined quantum time and is queued again because the process is not completed.\n", current->id);
            enqueue(&cpu2_queue, current);
        } else {
            // Finish process if it completes within the quantum time
            time += current->burst;
            fprintf(out, "Process %s is assigned to CPU-2, its operation is completed and terminated.\n", current->id);
        }
    }
}

// Enqueue a process into a queue
void enqueue(Queue *queue, Proc *proc) {
    if (queue->tail == QUEUE_CAP - 1) {
        printf("Queue overflow!\n");
        return;
    }
    queue->tail++;
    queue->elements[queue->tail] = proc;
}

// Dequeue a process from a queue
Proc *dequeue(Queue *queue) {
    if (is_queue_empty(queue)) {
        printf("Queue underflow!\n");
        return NULL;
    }
    Proc *proc = queue->elements[queue->head];
    queue->head++;
    return proc;
}

// Check if a queue is empty
int is_queue_empty(Queue *queue) {
    return (queue->head > queue->tail);
}

// Check if resources are available for a process
int check_resources(int current_ram_usage, int ram_required, int cpu_usage, int cpu_required) {
    if (current_ram_usage + ram_required <= MEM_SIZE) {
        return 1;
    }
    return 0;
}