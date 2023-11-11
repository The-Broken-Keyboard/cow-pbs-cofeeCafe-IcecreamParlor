#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int main(int argc,char* argv[]){
    if(argc != 3){
        fprintf(2,"setpriority: invalid number of arguments\n");
        exit(1);
    }
    int pid = atoi(argv[2]);
    int priority = atoi(argv[1]);
    int ret = setpriority(priority,pid);
    if(ret == -1){
        fprintf(2,"setpriority: unable to set priority\n");
        exit(1);
    }
    exit(0);
}