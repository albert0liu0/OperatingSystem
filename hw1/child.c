#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
int main(int argc,char* argv[]){
    int n=atoi(argv[1]);
    char* name=argv[2];
    for(int j=0;j<n;j++){
        volatile unsigned long i; for(i=0;i<1000000UL;i++);
    }
    printf("%s %d\n",name,getpid());
    exit(0);
}
