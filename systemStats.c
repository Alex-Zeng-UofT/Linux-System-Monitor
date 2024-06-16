#include <stdio.h>
#include <sys/utsname.h>

void printSystemStats(){
    printf("### System Information ### \n");
    struct utsname sInfo;
    uname(&sInfo);
    printf("System Name = %s\n", sInfo.sysname);
    printf("Machine Name = %s\n", sInfo.nodename);
    printf("Version = %s\n", sInfo.version);
    printf("Release = %s\n", sInfo.release);
    printf("Architecture = %s\n", sInfo.machine);
}