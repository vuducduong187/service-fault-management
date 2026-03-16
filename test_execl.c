#include<stdio.h>
#include<unistd.h>
#include <sys/wait.h>
int main(){
    int a = 5;
    pid_t pid = fork();
    if(pid < 0){
        printf("fault");
        return -1;
    }
    if(pid == 0){
        execl("./test", "test", NULL);

    }
    else if(pid > 0 ){
        printf("process parent\n");
        wait(NULL);
    }
    printf("ronaldo number one\n");
    return 0;
}