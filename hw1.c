#include "types.h"
#include "user.h"
int main(void){
    int count=0;
    int pid;
    printf(1,"Main(%d)\n",getpid());
    if(!(pid=fork())){
//        printf(1,"\tA[pid=%d]\t[getpid=%d]\n",pid,getpid());
        while((count<2)&&(pid=fork())){
            count++;
	    printf(1,"[A,ct=%d,pid=%d,getpid=%d]\n",count,pid,getpid());
        }
        if(count>0){
	    printf(1,"[B,ct=%d,pid=%d,getpid=%d]\n",count,pid,getpid());
        }
    }
    if(pid){
        waitpid(pid,0,0);
        count=count<<1;
	printf(1,"[C,ct=%d,pid=%d,getpid=%d]\n",count,pid,getpid());
    }
    exit(0);
}
