#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


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

typedef struct {
    char id[5];
    int ram;
    int priority;
    int arrival;
    int burst;
    int cpu;
} Proc;

typedef struct {
    Proc *elements[QUEUE_CAP];
    int head, tail;
} Queue;



void schedule_sjf(Proc *procs, int count, FILE *out);
void schedule_fcfs(Proc *procs, int count, FILE *out);
void schedule_rr(Proc *procs, int count, int quantum, int priority, FILE *out);
void enqueue(Queue *queue, Proc *proc);
Proc *dequeue(Queue *queue);
int is_queue_empty(Queue *queue);
int check_resources(int current_ram_usage, int ram_required, int cpu_usage, int cpu_required);








int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen("output.txt", "w");

    if (input == NULL || output == NULL) {
        printf("Error opening files!\n");
        return 1;
    }

    Proc procs[MAX_PROCS];
    int proc_count = 0;

    while (fscanf(input, "%[^,],%d,%d,%d,%d,%d\n", procs[proc_count].id,
                  &procs[proc_count].arrival, &procs[proc_count].priority,
                  &procs[proc_count].burst, &procs[proc_count].ram,
                  &procs[proc_count].cpu) == 6) {
        proc_count++;
    }

    fclose(input);

    schedule_fcfs(procs, proc_count, output);
    schedule_sjf(procs, proc_count, output);
    schedule_rr(procs, proc_count, RR_QUANTUM_MED, PRIORITY_MED, output);
    schedule_rr(procs, proc_count, RR_QUANTUM_LOW, PRIORITY_LOW, output);

    fclose(output);

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


void schedule_fcfs(Proc *procs, int count, FILE *out) {
    Queue cpu1_queue = { .head = 0, .tail = -1 };
    int time = 0;
    int current_ram_usage = RESERVE_MEM_FOR_PRIORITY_ZERO;

    for (int i = 0; i < count; i++) {
        if (procs[i].priority == PRIORITY_ZERO && check_resources(current_ram_usage, procs[i].ram, procs[i].cpu, procs[i].burst)) {
            current_ram_usage += procs[i].ram;
            fprintf(out, "Process %s is queued to be assigned to CPU-1.\n", procs[i].id);
            enqueue(&cpu1_queue, &procs[i]);
        }
    }

    while (!is_queue_empty(&cpu1_queue)) {
        Proc *current = dequeue(&cpu1_queue);
        fprintf(out, "Process %s is assigned to CPU-1.\n", current->id);
        time += current->burst;
        fprintf(out, "Process %s is completed and terminated.\n", current->id);
    }
}