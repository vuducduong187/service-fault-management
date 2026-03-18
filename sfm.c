#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <time.h>
#include<sys/wait.h>
#include<signal.h>
#include<stdlib.h>
#define MAX_MODULE 2

typedef enum{
    SM = 1,
    PM
}module_type;

typedef struct{
    module_type type;
    char name[32];
    int max_crash;
    int crash_cnt;
    int time_limit;
    int fault_level;
    time_t first_crash;
    pid_t pid;
}Module;

int idx_module = 0;
Module module[MAX_MODULE]; 

int convert_time(char *s){
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

void read_user_cfg(){
    Module m;
    memset(&m, 0, sizeof(Module));
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
                m.time_limit = convert_time(tmp_time);
                m.type = PM;
                module[idx_module++] = m;
            }
            fclose(f_default);
        }
    }
    else{
        while(fscanf(f, "%s", m.name)!= -1){
            fscanf(f, "%s%d%s%s", tmp, &m.max_crash, tmp, tmp_time);
            m.time_limit = convert_time(tmp_time);
            m.type = PM;
            module[idx_module++] = m;
        }
        fclose(f);
    }
}

void add_service_module(char *name){
    Module Sm;
    memset(&Sm, 0, sizeof(Module));
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
    else{
        module[idx_mdl].pid = pid;
    }
}

void start_all(){
    for(int i = 0; i < idx_module; i++){
        start_module(i);
    }
}

void restart_module(int idx_mdl){
    start_module(idx_mdl);
}

int find_module(pid_t PID){
    for(int i = 0; i < idx_module; i++){
        if(module[i].pid == PID){
            return i;
        }
    }
    return -1;
}

void handle_crash(int idx_mdl){
    if(module[idx_mdl].type == PM){
        module[idx_mdl].crash_cnt++;
        if(module[idx_mdl].crash_cnt > module[idx_mdl].max_crash && .................){
            module[idx_mdl].fault_level++;
            if(module[idx_mdl].fault_level < 2){
                restart_module(idx_mdl);
            }
        }

    }
}


void sig_handler(int signum){
    pid_t tmp_pid;
    int status;
    while((tmp_pid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("Khoi dong lai!\n");
        int tmp_idx_module = find_module(tmp_pid);
        if(tmp_idx_module >= 0){
            restart_module(tmp_idx_module);
        }
    }
}

int main(){
    signal(SIGCHLD, sig_handler);
    read_user_cfg();
    //add_service_module("Sm_1");
    start_all();
    while(1){
        pause();
    }
    return 0;
}
