#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}
int change_priority(int new,struct proc* prc)
{
  prc->running_time=0;
  prc->sleep_time=0;
  int op = prc->st_priority;
  prc->st_priority = new;
  release(&prc->lock);
  return op;
}
int check_valid_newp(int new)
{
  if(new<0 || new>100)
  return 1;
  return 0;
}
int check_valid_pid(int pid)
{
  if(pid<0)
  return 1;
  return 0;
}
uint64
sys_setpriority(void){

#ifdef PBS
  int new;
   int pid;
  argint(0,&pid);
  argint(1,&new);
  if(check_valid_newp(new)||check_valid_pid(pid)){
    return -1;
  }
  struct proc* prc=0;
  for( struct proc* p=proc;p<&proc[NPROC];p++){
    if(p->pid == pid){
      acquire(&p->lock);
      prc=p;
      break;
    }
  }
  if(!prc){
    return -1;
  }  
  return change_priority(new,prc);

#endif
#ifndef PBS
  return -1;
#endif
}