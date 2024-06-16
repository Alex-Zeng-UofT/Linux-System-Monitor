#include <stdio.h>
#include <sys/resource.h>
#include "userStats.c"
#include "systemStats.c"
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <utmp.h>
#include "cpuStats.c"

double absval(double num){
    if(num < 0)
        return 0-num;
    else
        return num;
}

int isNumeric(char *str){
    for(int i = 0; str[i] != '\0'; i++){
        if(!(str[i] > 47 && str[i] < 58)){
            return 0;
        }
    }
    return 1;
}

int getNumericValue(char *str){
    int sum = 0;
    int tens = 1;
    for(int i = strlen(str) - 1; i >= 0; i--){
        sum += (str[i] - 48) * tens;
        tens *= 10;
    }
    return sum;
}

float kiloToGiga(long kilobytes){
    return (float)kilobytes / (1000*1000*1000);
}

int main(int argc, char *argv[]){
    int samples = 10, freq = 1;
    int user = 0, systems = 0, sequential = 0, graphics = 0;
    if(argc >= 3){
        if(isNumeric(argv[2]) && isNumeric(argv[1])){
            if(getNumericValue(argv[2]) > 0){
                freq = getNumericValue(argv[2]);
            }
            if(getNumericValue(argv[1]) > 0){
                samples = getNumericValue(argv[1]);
            }
        }
    }
    else if(argc >= 2){
        if(isNumeric(argv[1]) && getNumericValue(argv[1]) > 0)
            samples = getNumericValue(argv[1]);
    }
    for(int i = 1; i < argc; i++){
        if(strncmp(argv[i], "--samples=", 10) == 0 && isNumeric((argv[i])+10) && getNumericValue((argv[i])+10) > 0){
            samples = getNumericValue((argv[i])+10);
        }
        else if(strncmp(argv[i], "--tdelay=", 9) == 0 && isNumeric((argv[i])+9) && getNumericValue((argv[i])+9) > 0){
             freq = getNumericValue((argv[i])+9);
        }
        else if(strcmp(argv[i], "--user") == 0){
            user = 1;
        }
        else if(strcmp(argv[i], "--system") == 0){
            systems = 1;
        }
        else if(strcmp(argv[i], "--sequential") == 0){
            sequential = 1;
        }
        else if(strcmp(argv[i], "--graphics") == 0 || strcmp(argv[i], "-g") == 0){
            graphics = 1;
        }
    }
    
    if(user == systems && !sequential && !graphics){
        float mem_used[samples];
        double cpu_usg[samples];
        struct rusage usage;
        getrusage(RUSAGE_SELF,&usage);
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 0; i < samples; i++){
            system("clear");
            printf("Nbr of samples: %d -- every %d secs\n", samples, freq);
            printf("-------------------------------------\n");
            printf("Memory usage: %ld kilobytes                         |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)   |     ### CPU ### \n");
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i] = pTotal - pFree;
            store_info_to_struct(&cur);
            cpu_usg[i] = get_cpu_usage(&prev, &cur);;
            for(int j = 0; j < i + 1; j++){
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB           |  cpu usage: %.2f%%\n", mem_used[j], pTotal, mem_used[j], pTotal + kiloToGiga(sInfo.totalswap), absval(cpu_usg[j]));
            }
            printf("                                                     | Total cpu use = %.2f%%\n", cpu_usg[i]);
            printf("---------------------------------------\n");
            printUserStats();
            printSystemStats();
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(user == systems && !sequential && graphics){
        float mem_used[samples];
        double cpu_usg[samples];
        struct rusage usage;
        getrusage(RUSAGE_SELF,&usage);
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 0; i < samples; i++){
            system("clear");
            printf("Nbr of samples: %d -- every %d secs\n", samples, freq);
            printf("-------------------------------------\n");
            printf("Memory usage: %ld kilobytes                                       |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)                 |     ### CPU ### \n");
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i] = pTotal - pFree;
            store_info_to_struct(&cur);
            cpu_usg[i] = get_cpu_usage(&prev, &cur);;
            for(int j = 0; j < i + 1; j++){
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB |", mem_used[j], pTotal, mem_used[j], pTotal + kiloToGiga(sInfo.totalswap));
                if(j==0){
                    printf("0.00 (%.2f)", mem_used[j]);
                }
                else{
                    int diff = 100 * (mem_used[j] - mem_used[j-1]);
                    if(diff < 0){
                        for(int k = 0; k < 0 - diff; k++)
                            printf(":");
                        printf("@ %.2f (%.2f)", absval(mem_used[j] - mem_used[j-1]), mem_used[j]);
                    }
                    else if(diff > 0){
                        for(int k = 0; k < diff; k++)
                            printf("#");
                        printf("* %.2f (%.2f)", absval(mem_used[j] - mem_used[j-1]), mem_used[j]);
                    }
                    else{
                        printf("0.00 (%.2f)", mem_used[j]);
                    }
                }
                printf("           |    ||");
                for(int k = 0; k < (int)(absval(cpu_usg[j])+1); k++){
                    printf("|");
                }
                printf("%.2f%%\n", absval(cpu_usg[j]));
            }
            printf("                                                                   | Total cpu use = %.2f%%\n", cpu_usg[i]);
            printf("---------------------------------------\n");
            printUserStats();
            printSystemStats();
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(user && !systems && !sequential){
        printf("Nbr of samples: %d -- every %d secs\n", samples, freq);
        printf("-------------------------------------\n");
        for(int i = 0; i < samples; i++){
            system("clear");
            printUserStats();
            printSystemStats();
            sleep(freq);
        }
    }
    else if(!user && systems && !sequential && !graphics){
        float mem_used[samples];
        double cpu_usg[samples];
        struct rusage usage;
        getrusage(RUSAGE_SELF,&usage);
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 0; i < samples; i++){
            system("clear");
            printf("Nbr of samples: %d -- every %d secs\n", samples, freq);
            printf("-------------------------------------\n");
            printf("Memory usage: %ld kilobytes                         |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)   |     ### CPU ### \n");
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i] = pTotal - pFree;
            store_info_to_struct(&cur);
            cpu_usg[i] = get_cpu_usage(&prev, &cur);;
            for(int j = 0; j < i + 1; j++){
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB           |  cpu usage: %.2f%%\n", mem_used[j], pTotal, mem_used[j], pTotal + kiloToGiga(sInfo.totalswap), absval(cpu_usg[j]));
            }
            printf("                                                     | Total cpu use = %.2f%%\n", cpu_usg[i]);
            printf("---------------------------------------\n");
            printSystemStats();
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(!user && systems && !sequential && graphics){
        float mem_used[samples];
        double cpu_usg[samples];
        struct rusage usage;
        getrusage(RUSAGE_SELF,&usage);
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 0; i < samples; i++){
            system("clear");
            printf("Nbr of samples: %d -- every %d secs\n", samples, freq);
            printf("-------------------------------------\n");
            printf("Memory usage: %ld kilobytes                                       |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)                 |     ### CPU ### \n");
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i] = pTotal - pFree;
            store_info_to_struct(&cur);
            cpu_usg[i] = get_cpu_usage(&prev, &cur);
            for(int j = 0; j < i + 1; j++){
                printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB |", mem_used[j], pTotal, mem_used[j], pTotal + kiloToGiga(sInfo.totalswap));
                if(j==0){
                    printf("0.00 (%.2f)", mem_used[j]);
                }
                else{
                    int diff = 100 * (mem_used[j] - mem_used[j-1]);
                    if(diff < 0){
                        for(int k = 0; k < 0 - diff; k++)
                            printf(":");
                        printf("@ %.2f (%.2f)", absval(mem_used[j] - mem_used[j-1]), mem_used[j]);
                    }
                    else if(diff > 0){
                        for(int k = 0; k < diff; k++)
                            printf("#");
                        printf("* %.2f (%.2f)", absval(mem_used[j] - mem_used[j-1]), mem_used[j]);
                    }
                    else{
                        printf("0.00 (%.2f)", mem_used[j]);
                    }
                }
                printf("           |    ||");
                for(int k = 0; k < (int)(absval(cpu_usg[j])+1); k++){
                    printf("|");
                }
                printf("%.2f%%\n", absval(cpu_usg[j]));
            }
            printf("                                                                   | Total cpu use = %.2f%%\n", cpu_usg[i]);
            printf("---------------------------------------\n");
            printSystemStats();
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(user == systems && sequential && !graphics){
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 1; i < samples + 1; i++){
            printf(">>> iteration %d\n", i);
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            printf("Memory usage: %ld kilobytes                         |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)   |     ### CPU ### \n");
            store_info_to_struct(&cur);
            double cpu_u = get_cpu_usage(&prev, &cur);;
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB           |  cpu usage: %.2f%%\n", pTotal-pFree, pTotal, pTotal-pFree, pTotal + kiloToGiga(sInfo.totalswap), cpu_u);
            printf("                                                     | Total cpu use = %.2f%%\n", cpu_u);
            printf("-------------------------------------\n");
            printUserStats();
            printSystemStats();
            printf("-------------------------------------\n\n");
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(user == systems && sequential && graphics){
        cpu prev, cur;
        float mem_used[samples];
        double cpu_usg[samples];
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 1; i < samples + 1; i++){
            printf(">>> iteration %d\n", i);
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            printf("Memory usage: %ld kilobytes                                      |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)                |     ### CPU ### \n");
            store_info_to_struct(&cur);
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i-1] = pTotal - pFree;
            cpu_usg[i-1] = get_cpu_usage(&prev, &cur);
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB |", mem_used[i-1], pTotal, mem_used[i-1], pTotal + kiloToGiga(sInfo.totalswap));
            if(i==1){
                printf("0.00 (%.2f)", mem_used[i-1]);
            }
            else{
                int diff = 100 * (mem_used[i-1] - mem_used[i-2]);
                if(diff < 0){
                    for(int k = 0; k < 0 - diff; k++)
                        printf(":");
                    printf("@ %.2f (%.2f)", absval(mem_used[i-1] - mem_used[i-2]), mem_used[i-1]);
                }
                else if(diff > 0){
                    for(int k = 0; k < diff; k++)
                        printf("#");
                    printf("* %.2f (%.2f)", absval(mem_used[i-1] - mem_used[i-2]), mem_used[i-1]);
                }
                else{
                    printf("0.00 (%.2f)", mem_used[i-1]);
                }
            }
            printf("           |    ||");
            for(int k = 0; k < (int)(absval(cpu_usg[i-1])+1); k++){
                printf("|");
            }
            printf("%.2f%%\n", absval(cpu_usg[i-1]));
            printf("-------------------------------------\n");
            printUserStats();
            printSystemStats();
            printf("-------------------------------------\n\n");
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(user && !systems && sequential){
        for(int i = 1; i < samples + 1; i++){
            printf(">>> iteration %d\n", i);
            printUserStats();
            printSystemStats();
            printf("-------------------------------------\n\n");
            sleep(freq);
        }
    }
    else if(!user && systems && sequential && !graphics){
        cpu prev, cur;
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 1; i < samples + 1; i++){
            printf(">>> iteration %d\n", i);
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            printf("Memory usage: %ld kilobytes                         |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)   |     ### CPU ### \n");
            store_info_to_struct(&cur);
            double cpu_u = get_cpu_usage(&prev, &cur);;
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB           |  cpu usage: %.2f%%\n", pTotal-pFree, pTotal, pTotal-pFree, pTotal + kiloToGiga(sInfo.totalswap), cpu_u);
            printf("                                                     | Total cpu use = %.2f%%\n", cpu_u);
            printf("-------------------------------------\n");
            printSystemStats();
            printf("-------------------------------------\n\n");
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    else if(!user && systems && sequential && graphics){
        cpu prev, cur;
        float mem_used[samples];
        double cpu_usg[samples];
        store_info_to_struct(&prev);
        sleep(freq);
        for(int i = 1; i < samples + 1; i++){
            printf(">>> iteration %d\n", i);
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            printf("Memory usage: %ld kilobytes                                      |  Number of cores: %ld\n", usage.ru_maxrss, sysconf(_SC_NPROCESSORS_ONLN));
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)                |     ### CPU ### \n");
            store_info_to_struct(&cur);
            struct sysinfo sInfo;
            sysinfo(&sInfo);
            float pFree = kiloToGiga(sInfo.freeram);
            float pTotal = kiloToGiga(sInfo.totalram);
            mem_used[i-1] = pTotal - pFree;
            cpu_usg[i-1] = get_cpu_usage(&prev, &cur);
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB |", mem_used[i-1], pTotal, mem_used[i-1], pTotal + kiloToGiga(sInfo.totalswap));
            if(i==1){
                printf("0.00 (%.2f)", mem_used[i-1]);
            }
            else{
                int diff = 100 * (mem_used[i-1] - mem_used[i-2]);
                if(diff < 0){
                    for(int k = 0; k < 0 - diff; k++)
                        printf(":");
                    printf("@ %.2f (%.2f)", absval(mem_used[i-1] - mem_used[i-2]), mem_used[i-1]);
                }
                else if(diff > 0){
                    for(int k = 0; k < diff; k++)
                        printf("#");
                    printf("* %.2f (%.2f)", absval(mem_used[i-1] - mem_used[i-2]), mem_used[i-1]);
                }
                else{
                    printf("0.00 (%.2f)", mem_used[i-1]);
                }
            }
            printf("           |    ||");
            for(int k = 0; k < (int)(absval(cpu_usg[i-1])+1); k++){
                printf("|");
            }
            printf("%.2f%%\n", absval(cpu_usg[i-1]));
            printf("-------------------------------------\n");
            printSystemStats();
            printf("-------------------------------------\n\n");
            store_info_to_struct(&prev);
            sleep(freq);
        }
    }
    return 0;
}

