#include<string.h>
#include<stdio.h>
#include<stdlib.h>
struct processdata{
    char* name;
    int readyTime;
    int executionTime;
};
typedef struct processdata processData;
struct processlist{
    int number;
    processData* list;
};
typedef struct processlist processList;
void timeUnit(int n){
    for(int j=0;j<n;j++)
        volatile unsigned long i; for(i=0;i<1000000UL;i++);
}
int scheduleByFIFO(processList pl){
    
}
int scheduleByRR(processList pl){
}
int scheduleBySJF(processList pl){
}
int scheduleByPSJF(processList pl){
}
int cmp(void* a,void* b){
    list* la=(list*)a;
    list* lb=(list*)b;
    if((la->readyTime)<(lb->readyTime))return -1;
    if((la->readyTime)>(lb->readyTime))return 1;
    return 0;

}
int main(){
    char schedulePolicy[5];
    scanf("%s",schedulePolicy);
    processList pl;
    scanf("%d",&pl.number);
    pl.list=malloc(sizeof(processData)*pl.number);
    for(int i=0;i<pl.number;i++){
        pl.list[i].name=malloc(sizeof(char)*40);
        scanf("%s",pl.list[i].name);
        scanf("%d",&pl.list[i].readyTime);
        scanf("%d",&pl.list[i].executionTime);
    }
    qsort(pl.list,pl.list+pl.number,cmp);
    if(strcmp(schedulePolicy,"FIFO")==0)scheduleByFIFO(pl);
    if(strcmp(schedulePolicy,"RR")==0)scheduleByRR(pl);
    if(strcmp(schedulePolicy,"SJF")==0)scheduleBySJF(pl);
    if(strcmp(schedulePolicy,"PSJF")==0)scheduleByPSJF(pl);
}
