#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// my defines and inlcudes

#define MAX_HISTORY 10
#define MAX_COMMAND_LEN 100

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// my code starts here
// extern struct proc proc[NPROC]; // Process table

// int sys_gethistory(void) {
//     struct proc *p;
//     printf("PID\tProcess Name\tMemory\n");

//     for (p = proc; p < &proc[NPROC]; p++) {
//         if (p->state != UNUSED) {
//             printf("%d\t%s\t%d\n", p->pid, p->name, p->sz);
//         }
//     }
//     return 0;
// }


// my code starts here

char command_history[MAX_HISTORY][MAX_COMMAND_LEN];
int history_count = 0;

int
sys_gethistory(void)
{
  int n;
  char *buf;
  
  if(argint(0, &n) < 0 || argptr(1, &buf, MAX_COMMAND_LEN) < 0)
    return -1;
    
  if(n < 0 || n >= history_count)
    return -1;
    
  strncpy(buf, command_history[n], MAX_COMMAND_LEN);
  return 0;
}

// In sysproc.c
int
sys_addhistory(void)
{
  char *cmd;
  
  if(argptr(0, &cmd, MAX_COMMAND_LEN) < 0)
    return -1;
    
  // Shift history if we've reached the maximum
  if(history_count == MAX_HISTORY) {
    for(int i = 0; i < MAX_HISTORY - 1; i++) {
      strncpy(command_history[i], command_history[i+1], MAX_COMMAND_LEN);
    }
    history_count--;
  }
  
  // Add new command to history
  strncpy(command_history[history_count], cmd, MAX_COMMAND_LEN);
  history_count++;
  
  return 0;
}