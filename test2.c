#include<stdio.h>
#include<unistd.h>
int main(){
    for(int i = 0; i < 5; i++){
        printf("arsenal toi nay thua\n");
        sleep(5);
    }
    while(1){
        int *ptr = NULL;
        *ptr = 100;
        int arr[5] = {1,2,3,4,5};
        arr[10] = 999; 
        sleep(3);
    }
    return 0;
}