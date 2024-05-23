##PROJECT DESCRIPTION

This code is a simulation of an operating system task scheduler. It manages processes with different priorities using various scheduling algorithms like First-Come, First-Served (FCFS), Shortest Job First (SJF), and Round Robin (RR).

- [FCFS (First-Come, First-Served):](https://www.geeksforgeeks.org/first-come-first-serve-cpu-scheduling-non-preemptive/)  Processes are executed in the order they arrive, regardless of their priority.
- [SJF (Shortest Job First):](https://www.geeksforgeeks.org/program-for-shortest-job-first-or-sjf-cpu-scheduling-set-1-non-preemptive/)  Processes with the shortest burst time are executed first, promoting faster turnaround.
- [Round Robin:](https://www.geeksforgeeks.org/program-for-round-robin-scheduling-for-the-same-arrival-time/) Processes are executed for a fixed time quantum, allowing fair CPU time allocation among processes.
The code reads process information from an input file and schedules them using the mentioned algorithms. It then writes the scheduling results to an output file. Additionally, it prints the CPU queues for each priority level to the console.

The main purpose of this code is to demonstrate how different scheduling algorithms work in managing system processes and CPU utilization.
