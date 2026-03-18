#include<stdio.h>
#include<unistd.h>
int main(){
    for(int i = 0; i < 5; i++){
        printf("real madrid toi nay thang \n");
        sleep(5);
    }
    while(1){
        int x = 1/0;
        printf("%d", x);
        int *ptr = NULL;
        *ptr = 100;
        sleep(3);
    }
    return 1;
}