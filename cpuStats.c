#include <stdio.h>
#include <unistd.h>

typedef struct cpu {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
}cpu;

void store_info_to_struct(cpu *pro){
    FILE *cpu_stats = fopen("/proc/stat", "r");
    char name[10];
    fscanf(cpu_stats, "%s %ld %ld %ld %ld %ld %ld %ld", name, &(pro->user), &(pro->nice), &(pro->system), &(pro->idle), &(pro->iowait), &(pro->irq), &(pro->softirq));
    fclose(cpu_stats);
	return;
}

double get_total_usage(cpu *cpu){
    return (cpu->user) + (cpu->nice) + (cpu->system) + (cpu->idle) + (cpu->iowait);+ (cpu->irq) + (cpu->softirq);
}

double get_cpu_usage(cpu *prev, cpu *cur){
    double free_space = (cur->idle) + (cur->iowait) - (prev->idle) - (prev->iowait);
    return (1000 * (get_total_usage(cur) - get_total_usage(prev) - free_space) / (get_total_usage(cur) - get_total_usage(prev)) + 1) / 10;
}
