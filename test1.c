#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define PORT 8888 

void daemonize(){
    pid_t pid = fork();
    if(pid < 0) exit(1);
    if(pid > 0) exit(0);

    setsid();

    pid = fork();
    if(pid < 0) exit(1);
    if(pid > 0) exit(0);

    chdir("/");
    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int sock;
void setup_client(){
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "192.168.160.110", &server.sin_addr) <= 0) {
        return;
    }

    if(connect(sock,(struct sockaddr*)&server,sizeof(server)) < 0){
        exit(1);
    }

    send(sock,"Pm_1",sizeof("Pm_1"),0);

   // while(1){
       // send(sock,"Alive",sizeof("Alive"),0);
       // sleep(1);
    //}
}

int main(){
    setup_client();
    while(1){
        send(sock,"Alive",5,0);
        for(int i = 0; i < 2; i++){
            printf("real madrid toi nay thang \n");
            sleep(1);
        }
        int *ptr = NULL;
        *ptr = 100;
    }
    return 1;
}
