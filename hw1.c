#include "types.h"
#include "user.h"

int main(){
    int count=0;
    int pid;
    if(!(pid=fork())){
        while((count<2)&&(pid=fork())){
            count++;
            printf(1,"%d",count);
        }
        if(count>0){
            printf(1,"%d",count);
        }
    }
    if(pid){
        waitpid(pid,0,0);
        count=count<<1;
        printf(1,"%d",count);
    }
}
