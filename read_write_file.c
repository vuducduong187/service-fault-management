#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <time.h>
#define MAX_MODULE 20
typedef struct{
    char name[32];
    int max_crash;
    int crash_cnt;
    int time_limit;
    int fault_level;
    time_t first_crash;
    pid_t pid;
}Module;
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
    FILE *f;
    f = fopen("sfm_user_cfg.conf", "r");
    int idx_module = 0;
    Module module[MAX_MODULE];
    Module m;
    memset(&m, 0, sizeof(Module));
    char tmp[10];
    char tmp_time[15];
    while(fscanf(f, "%s", m.name)!= -1){
        fscanf(f, "%s%d%s%s", tmp, &m.max_crash, tmp, tmp_time);
        m.time_limit = convert_time(tmp_time);
        module[idx_module++] = m;
    }
    for(int i = 0; i < idx_module; i++){
        printf("%s %d %d %d %d", module[i].name, module[i].max_crash, module[i].crash_cnt, module[i].time_limit, m.pid);
        printf("\n");
    }
    fclose(f);
}
int main(){
    read_user_cfg();
    return 0;
}