#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <time.h>
#include<sys/wait.h>
#include<signal.h>
#include<stdlib.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>

#define MAX_MODULE 2
#define PORT 8888

typedef enum{
    SM = 1,
    PM
}module_type;

typedef enum{
    BY_USER,
    By_SFM
}rstbysfm;

typedef struct{
    module_type type;
    char name[32];
    int max_crash;
    int crash_cnt;
    int time_limit;
    int fault_level;
    time_t time_start_mdl;
    //pid_t pid;
    int sock_fd;
    time_t last_recv_respone;
}Module;

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


int idx_module = 0;
Module module[MAX_MODULE]; 

int convert_stime2second(char *s){
    int num = 0;
    int total = 0;
    for(int i = 0; s[i] != '\0'; i++){
        if(s[i] >= '0' && s[i] <= '9'){
            num = num*10 + (s[i]-'0');
        }
        else{
            if(s[i] == 'd')
                total += num * 86400;
            else if(s[i] == 'h')
                total += num * 3600;
            else if(s[i] == 'm')
                total += num * 60;
            else if(s[i] == 's')
                total += num;
            num = 0;
        }
    }
    return total;
}

void convert_second2stime(char tmp[5], char dest[15], int seconds){
    if(seconds >= 86400){
        int d = seconds/86400;
        sprintf(tmp, "%dd", d);
        strcat(dest, tmp);
        seconds%=86400;
    }
    if(seconds >= 3600){
        int h = seconds/3600;
        sprintf(tmp, "%dh", h);
        strcat(dest, tmp);
        seconds%=3600;
    }
    if(seconds >= 60){
        int m = seconds/60;
        sprintf(tmp, "%dm", m);
        strcat(dest, tmp);
        seconds%=60;
    }
    if(seconds != 0){
        int s = seconds;
        sprintf(tmp, "%ds", s);
        strcat(dest, tmp);
    }
}

void read_user_cfg(){
    Module m;
    memset(&m, 0, sizeof(Module));
    m.sock_fd = -1;

    char tmp[10];
    char tmp_time[15];
    FILE *f;
    f = fopen("sfm_user_cfg.conf", "r");
    if(!f){
        pid_t pid = fork();
        if(pid < 0){
            return;
        }
        if(pid == 0){
            execl("/usr/bin/touch", "touch", "sfm_default_cfg.conf", NULL);
            exit(1);
        }
        else{
            wait(NULL);
            FILE *f_default;
            f_default = fopen("sfm_default_cfg.conf", "a");
            for(int i = 1; i <= MAX_MODULE; i++){
                char tmp_name[32];
                memset(tmp_name, 0, sizeof(tmp_name));
                char buf[20];
                memset(buf, 0, sizeof(buf));
                int tmp = i;
                sprintf(buf, "%d",tmp);
                char Pm[10] = "Pm_";
                strcat(Pm,buf);
                strcpy(tmp_name, Pm);
                fprintf(f_default, "%s\n  %s %d %s %s\n", tmp_name, "crashed", 5, "time", "2h");
                fflush(f_default);
            }
            fclose(f_default);
            f_default = fopen("sfm_default_cfg.conf", "r");
            while(fscanf(f_default, "%s", m.name)!= -1){
                fscanf(f_default, "%s%d%s%s", tmp, &m.max_crash, tmp, tmp_time);
                m.time_limit = convert_stime2second(tmp_time);
                m.type = PM;
                module[idx_module++] = m;
            }
            fclose(f_default);
        }
    }
    else{
        while(fscanf(f, "%s", m.name)!= -1){
            fscanf(f, "%s%d%s%s", tmp, &m.max_crash, tmp, tmp_time);
            m.time_limit = convert_stime2second(tmp_time);
            m.type = PM;
            module[idx_module++] = m;
        }
        fclose(f);
    }
}

void add_service_module(char *name){
    Module Sm;
    memset(&Sm, 0, sizeof(Module));
    Sm.sock_fd = -1;
    Sm.type = SM;
    strcpy(Sm.name, name);
    Sm.max_crash = 1;
    module[idx_module++] = Sm;
}

void start_module(int idx_mdl){
    pid_t pid = fork();
    if(pid < 0){
        return;
    }
    if(pid == 0){
        char path[50];
        sprintf(path, "./%s", module[idx_mdl].name);
        execl(path, module[idx_mdl].name, NULL);
        exit(1);
    }
}

void start_all(){
    for(int i = 0; i < idx_module; i++){
        sleep(1);
        start_module(i);
        module[i].time_start_mdl = time(NULL);
    }
}

void restart_module(int idx_mdl){
    printf("Module %s Crashed\n", module[idx_mdl].name);
    start_module(idx_mdl);
}

void reboot_device(){
    pid_t pid = fork();
    if(pid < 0) return;
    if(pid == 0){
        execl("/usr/bin/rm", "rm", "sfm_cur_stt.conf", NULL);
        exit(1);
    }
    else{
        wait(NULL);
        sync();
        sleep(1);
        reboot(RB_AUTOBOOT);
    }
}

void write_cur_stt(int idx_mdl){
    FILE *f_curr_stt;
    f_curr_stt = fopen("sfm_cur_stt.conf", "a");
    if(!f_curr_stt) return;
    char tmp[5];
    memset(tmp, 0, sizeof(tmp));
    char dest[15];
    memset(dest, 0, sizeof(dest));
    convert_second2stime(tmp, dest, (time(NULL)- module[idx_mdl].time_start_mdl));
    fprintf(f_curr_stt, "%s\n %s\n %s %d %s %s\n", "rstbysfm", module[idx_mdl].name, "crashed", module[idx_mdl].crash_cnt, "time", dest);
    if(module[idx_mdl].fault_level >= 1){
        fprintf(f_curr_stt, "%s %d\n", "fault", module[idx_mdl].fault_level);
    }
    fclose(f_curr_stt);
}

void handle_crash(int idx_mdl){
    if(module[idx_mdl].type == PM){
        module[idx_mdl].crash_cnt++;
        if(module[idx_mdl].crash_cnt <= module[idx_mdl].max_crash && ((time(NULL) - module[idx_mdl].time_start_mdl) <= module[idx_mdl].time_limit)){
            write_cur_stt(idx_mdl);//cap nhat
            restart_module(idx_mdl);
            //cap nhat crash_cnt va crash_time vao file sfm_cur_stt.conf
        }
        if(module[idx_mdl].crash_cnt > module[idx_mdl].max_crash && ((time(NULL) - module[idx_mdl].time_start_mdl) <= module[idx_mdl].time_limit)){
            module[idx_mdl].fault_level++;
            write_cur_stt(idx_mdl);
            //cap nhat fault_level vao file sfm_cur_stt.conf
            module[idx_mdl].crash_cnt = 0;
            if(module[idx_mdl].fault_level < 2){
                printf("Module %s Fault level 1\n", module[idx_mdl].name);
                restart_module(idx_mdl);
            }
            else{
                printf("Fault = 2 khong khoi dong lai nua\n");
                return;
            }
        }
        if(module[idx_mdl].crash_cnt <= module[idx_mdl].max_crash && ((time(NULL) - module[idx_mdl].time_start_mdl) > module[idx_mdl].time_limit)){
            module[idx_mdl].crash_cnt = 0;
            //xoa trang thai cap nhat trong file sfm_cur_stt.conf
        }
    }
    else{
        rstbysfm restart = By_SFM;
        reboot_device();
    }
}


int server_fd;
void setup_socket_sfmd(){
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return;

    if(listen(server_fd, MAX_MODULE) < 0) return;
}

void accept_new(){
    int client_fd;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    client_fd = accept(server_fd,(struct sockaddr*)&client,&len);

    char tmp_name[32]={0};
    recv(client_fd,tmp_name,sizeof(tmp_name),0);

    for(int i = 0; i < idx_module; i++){
        if(strcmp(module[i].name,tmp_name) == 0){
            if(module[i].sock_fd != -1){
                close(module[i].sock_fd);
            }
            module[i].sock_fd = client_fd;
            module[i].last_recv_respone = time(NULL);
        }
    }
}

void monitor(){
    char buf[32];
    while(1){
        fd_set set;
        FD_ZERO(&set);
        FD_SET(server_fd,&set);

        struct timeval tv = {0,0};

        if(select(server_fd+1,&set,NULL,NULL,&tv) > 0){
            accept_new();
        }

        for(int i = 0; i < idx_module; i++){
            if(module[i].sock_fd < 0) continue;
            int ret = recv(module[i].sock_fd,buf,sizeof(buf),MSG_DONTWAIT);
            if(ret == 0){
                close(module[i].sock_fd);
                module[i].sock_fd = -1;
                handle_crash(i);
            }
            else if(ret > 0){
                module[i].last_recv_respone = time(NULL);
            }
            else{
                // ret < 0
                if(errno != EAGAIN && errno != EWOULDBLOCK){
                    // lỗi thật sự → coi như crash
                    close(module[i].sock_fd);
                    module[i].sock_fd = -1;
                    handle_crash(i);
                }
            }
            if(module[i].last_recv_respone != 0 && time(NULL) - module[i].last_recv_respone > 5){
                close(module[i].sock_fd);
                module[i].sock_fd = -1;
                handle_crash(i);
            }
        }
        sleep(1);
    }
}

int main(){
    read_user_cfg();
    setup_socket_sfmd();
    start_all();
    monitor();
    //add_service_module("Sm_1");
    //while(1){
        //pause();
    //}
    return 0;
}