#include <stdio.h>
#include <utmp.h>

void printUserStats(){
    printf("### Sessions/users ### \n");
    struct utmp *uSession;
    setutent();
    while((uSession = getutent()) != NULL){
        if(uSession->ut_type == USER_PROCESS){
            printf("%s %s (%s)\n", uSession->ut_user, uSession->ut_line, uSession->ut_host);
        }
    }
    endutent();
    printf("---------------------------------------\n");
}