#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<wait.h>
#include<signal.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sched.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define HIGH -20
#define LOW 19
#define MAX (1<<25)
#define UNIT 1000000UL

struct processdata{
    char* name;
    int readyTime;
    int executionTime;
    int startTime;
    int pid;
};
typedef struct processdata processData;
struct processlist{
    int number;
    processData* list;
};

void start_child(const int prio, processData p) {
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1,&cpuSet);
    sched_setaffinity(0,sizeof(cpuSet),&cpuSet);

    setpriority(PRIO_PROCESS, getpid(), prio);
    char arg[10];
    sprintf(arg, "%d", p.executionTime);
    execlp("./child", "./child", arg, p.name, NULL);

}

void run() {
    volatile unsigned long i;
    for(i=0; i<UNIT; i++);
}

void swap(processData *a, processData *b) {
    processData tmp=*a;
    *a=*b;
    *b=tmp;
}

int flag=0;

typedef struct processlist processList;

int scheduleByFIFO(processList pl){
    int st=0, ed=0;
    for(int t=0; t<MAX && st<pl.number; t++) {
        if(ed < pl.number && pl.list[ed].readyTime <= t) {
            //fprintf(stderr, "PUSH:\t%d %d | t = %d\n", st, ed, t);
            int pid;
            if((pid=fork()) == 0)
                start_child((st==ed)?HIGH:LOW, pl.list[ed]);
            pl.list[ed].pid = pid;
            ed++;
        }
        int res = waitpid(pl.list[st].pid, NULL, WNOHANG);
        if(res > 0) {
            //fprintf(stderr, "POP:\t%d %d | t = %d\n", st, ed, t);
            st++;
            if(st < ed)
                setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
        }
        run();
    }
    return 0;
}
int scheduleByRR(processList pl){
    int st=0, ed=0, done=0, cnt=0;
    for(int t=0; t<MAX&&st<pl.number; t++) {
        if(ed < pl.number && pl.list[ed].readyTime <= t) {
            int pid;
            if((pid=fork()) == 0){
                fprintf(stderr,"!!!!!\n");
                start_child((st==ed)?HIGH:LOW, pl.list[ed]);
            }
            pl.list[ed].pid = pid;
            ed++;
        }
        int res = waitpid(pl.list[st].pid, NULL, WNOHANG);
        if(res >0) {
            fprintf(stderr,"%d\n",res);
            pl.list[st].executionTime=0;
            st++;
            done++;
            cnt=0;
            if(done < pl.number){
                while(st < ed &&  pl.list[st].executionTime== 0)
                    st=(st==ed-1)? 0:st+1;
                setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
            }
        }
        else if(res>=0&&cnt >= 500){
            setpriority(PRIO_PROCESS, pl.list[st].pid, LOW);
            st++;
            cnt=0;
            while(st < ed && pl.list[st].executionTime == 0)
                st=(st==ed-1)? 0:st+1;
            setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
        }
        run();
        cnt++;
    }
    while(done < pl.number){
        int res = waitpid(pl.list[st].pid, NULL, WNOHANG);
        if(res >0) {
            pl.list[st].executionTime=0;
            st++;
            done++;
            cnt=0;
            if(done < pl.number){
                while(st < ed &&  pl.list[st].executionTime== 0)
                    st=(st==ed-1)? 0:st+1;
                setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
            }
        }
        else if(res>=0&&cnt >= 500){
            setpriority(PRIO_PROCESS, pl.list[st].pid, LOW);
            st++;
            cnt=0;
            while(st < ed && pl.list[st].executionTime == 0)
                st=(st==ed-1)? 0:st+1;
            setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
        }
        run();
        cnt++;
    }
    return 0;
}
int scheduleBySJF(processList pl){
    int st=0, ed=0;
    for(int t=0; t<MAX && st<pl.number; t++) {
        if(ed < pl.number && pl.list[ed].readyTime <= t) {
            //fprintf(stderr, "PUSH:\t%d %d | t = %d\n", st, ed, t);
            int pid;
            if((pid=fork()) == 0)
                start_child((st==ed)?HIGH:LOW, pl.list[ed]);
            pl.list[ed].pid = pid;
            ed++;
        }
        int res = waitpid(pl.list[st].pid, NULL, WNOHANG);
        if(res > 0) {
            //fprintf(stderr, "POP:\t%d %d | t = %d\n", st, ed, t);
            st++;
            if(st < ed) {
                int nxt=st;
                for(int i=st+1; i<ed; i++)
                    if(pl.list[i].executionTime < pl.list[nxt].executionTime)
                        nxt=i;
                swap(pl.list+st, pl.list+nxt);
                setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
            }
        }
        run();
    }
    return 0;
}
int scheduleByPSJF(processList pl){
    int st=0, ed=0;
    for(int t=0; t<MAX && st<pl.number; t++) {
        if(ed < pl.number && pl.list[ed].readyTime <= t) {
            //fprintf(stderr, "PUSH:\t%d %d | t = %d\n", st, ed, t);
            if(st<ed && t - pl.list[st].startTime > pl.list[ed].executionTime) {
                setpriority(PRIO_PROCESS, pl.list[st].pid, LOW);
                pl.list[st].executionTime = t - pl.list[st].startTime;
                swap(pl.list+st, pl.list+ed);
                int pid;
                if((pid=fork()) == 0)
                    start_child(HIGH, pl.list[st]);
                pl.list[st].startTime = t;
                pl.list[st].pid = pid;
            } else {
                int pid;
                if((pid=fork()) == 0)
                    start_child((st==ed)?HIGH:LOW, pl.list[ed]);
                pl.list[ed].startTime = t;
                pl.list[ed].pid = pid;
            }
            ed++;
        }
        int res = waitpid(pl.list[st].pid, NULL, WNOHANG);
        if(res > 0) {
            //fprintf(stderr, "POP:\t%d %d | t = %d\n", st, ed, t);
            st++;
            if(st < ed) {
                int nxt=st;
                for(int i=st+1; i<ed; i++)
                    if(pl.list[i].executionTime < pl.list[nxt].executionTime)
                        nxt=i;
                swap(pl.list+st, pl.list+nxt);
                setpriority(PRIO_PROCESS, pl.list[st].pid, HIGH);
            }
        }
        run();
    }
    return 0;
}

int cmp(const void *a, const void *b) {
    int x = ((processData*)a)->readyTime;
    int y = ((processData*)b)->readyTime;
    if(x==y)
        return 0;
    else if(x<y)
        return -1;
    else
        return 1;
}

int main(){
    //fprintf(stderr, "CPU: %d\n", (int)sysconf(_SC_NPROCESSORS_CONF));
    setpriority(PRIO_PROCESS, getpid(), HIGH);
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(0,&cpuSet);
    sched_setaffinity(0,sizeof(cpuSet),&cpuSet);
    char schedulePolicy[5];
    scanf("%s",schedulePolicy);
    processList pl;
    scanf("%d",&pl.number);
    pl.list=(processData*)malloc(sizeof(processData)*pl.number);
    for(int i=0;i<pl.number;i++){
        pl.list[i].name=(char*)malloc(sizeof(char)*40);
        scanf("%s",pl.list[i].name);
        scanf("%d",&pl.list[i].readyTime);
        scanf("%d",&pl.list[i].executionTime);
    }
    qsort(pl.list, pl.number, sizeof(processData), cmp);
    if(strcmp(schedulePolicy,"FIFO")==0)scheduleByFIFO(pl);
    if(strcmp(schedulePolicy,"RR")==0)scheduleByRR(pl);
    if(strcmp(schedulePolicy,"SJF")==0)scheduleBySJF(pl);
    if(strcmp(schedulePolicy,"PSJF")==0)scheduleByPSJF(pl);

    return 0;
}
