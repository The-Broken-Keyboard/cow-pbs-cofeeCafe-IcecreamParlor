#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int main(int argc,char* argv[]){
    if(argc != 3){
        fprintf(2,"invalid input\n");
        exit(1);
    }
    int priority = atoi(argv[2]);
    int pid = atoi(argv[1]);
    
    if(setpriority(pid,priority == -1)){
        fprintf(2,"set priority error\n");
        exit(1);
    }
    exit(0);
}