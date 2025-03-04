



#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

// My defines and includes
#define MAX_HISTORY 10
struct spinlock history_lock;
struct history_entry history[MAX_HISTORY];
static int history_count = 0;


int sys_fork(void) {
  return fork();
}

int sys_exit(void) {
  exit();
  return 0;  // Not reached
}

int sys_wait(void) {
  return wait();
}

int sys_kill(void) {
  int pid;
  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void) {
  return myproc()->pid;
}

int sys_sbrk(void) {
  int addr;
  int n;
  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void) {
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

int sys_uptime(void) {
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// // My code starts here
// char command_history[MAX_HISTORY][100]; // Removed MAX_COMMAND_LEN usage
// int history_count = 0;

int sys_block(void) {
  int syscall_id;
  if(argint(0, &syscall_id) < 0)
    return -1;
  
  struct proc *curproc = myproc();
  curproc->blocked_syscalls[syscall_id] = 1;
  cprintf("Blocked syscall %d\n", syscall_id);
  
  return 0;
}

int sys_unblock(void) {
  int syscall_id;
  if(argint(0, &syscall_id) < 0)
    return -1;
  
  struct proc *curproc = myproc();
  curproc->blocked_syscalls[syscall_id] = 0;
  
  return 0;
}

// int sys_gethistory(void) {

//   return 0;
// }



// // Declare history and lock if missing
// struct spinlock history_lock;
// struct history_entry history[MAX_HISTORY];

// int sys_gethistory(void) {
//   struct history_entry *user_entries;
//   int user_max;
//   int i, count, entries_to_copy;

//   // Validate the user pointer for the entire buffer rather than just one entry
//   if(argptr(0, (char**)&user_entries, user_max * sizeof(struct history_entry)) < 0 ||
//      argint(1, &user_max) < 0) {
//     return -1;
//   }

//   if(user_max < 0) return -1;

//   // Acquire lock to protect global history
//   // acquire(&history_lock);
//   count = history_count;
//   if (count > MAX_HISTORY) count = MAX_HISTORY;

//   // Optionally, you can remove the temporary array and sort altogether
//   // since you don't want to sort by start_time.
//   // Directly copy the entries from the global history to user space.
//   entries_to_copy = (user_max < count) ? user_max : count;

//   for(i = 0; i < entries_to_copy; i++) {
//     if(copyout(myproc()->pgdir,
//                (uint)user_entries + i * sizeof(struct history_entry),
//                &history[i],
//                sizeof(struct history_entry)) < 0) {
//       // release(&history_lock);
//       return -1;
//     }
//   }

//   // release(&history_lock);
//   return entries_to_copy;
// }




// // Declare history and lock if missing
// struct spinlock history_lock;
struct history_entry history[MAX_HISTORY];

int sys_gethistory(void) {
  struct history_entry *user_entries;
  int user_max;
  struct history_entry tmp_entries[MAX_HISTORY];
  int i, count, entries_to_copy;

  // Fix `argptr` type mismatch (use `char**` instead of `void**`)
  if(argptr(0, (char**)&user_entries, sizeof(struct history_entry)) < 0 ||
     argint(1, &user_max) < 0) {
    return -1;
  }

  if(user_max < 0) return -1;

  acquire(&history_lock);  // Fix undeclared `history_lock`
  count = history_count;
  if (count > MAX_HISTORY) count = MAX_HISTORY;

  // Fix undeclared `history`
  for(i = 0; i < count; i++) tmp_entries[i] = history[i];

  // Sort by start_time (ascending)
  for(i = 0; i < count - 1; i++) {
    for(int j = 0; j < count - i - 1; j++) {
      if(tmp_entries[j].start_time > tmp_entries[j + 1].start_time) {
        struct history_entry temp = tmp_entries[j];
        tmp_entries[j] = tmp_entries[j + 1];
        tmp_entries[j + 1] = temp;
      }
    }
  }

  entries_to_copy = (user_max < count) ? user_max : count;

  // Copy to user space
  for(i = 0; i < entries_to_copy; i++) {
    if(copyout(myproc()->pgdir, (uint)user_entries + i * sizeof(struct history_entry),
               &tmp_entries[i], sizeof(struct history_entry)) < 0) {
      release(&history_lock);
      return -1;
    }
  }

  release(&history_lock);
  return entries_to_copy;
}









// #include "types.h"
// #include "x86.h"
// #include "defs.h"
// #include "date.h"
// #include "param.h"
// #include "memlayout.h"
// #include "mmu.h"
// #include "proc.h"
// #include "spinlock.h"

// // my defines and inlcudes

// #define MAX_HISTORY 10
// #define MAX_COMMAND_LEN 100

// int
// sys_fork(void)
// {
//   return fork();
// }

// int
// sys_exit(void)
// {
//   exit();
//   return 0;  // not reached
// }

// int
// sys_wait(void)
// {
//   return wait();
// }

// int
// sys_kill(void)
// {
//   int pid;

//   if(argint(0, &pid) < 0)
//     return -1;
//   return kill(pid);
// }

// int
// sys_getpid(void)
// {
//   return myproc()->pid;
// }

// int
// sys_sbrk(void)
// {
//   int addr;
//   int n;

//   if(argint(0, &n) < 0)
//     return -1;
//   addr = myproc()->sz;
//   if(growproc(n) < 0)
//     return -1;
//   return addr;
// }

// int
// sys_sleep(void)
// {
//   int n;
//   uint ticks0;

//   if(argint(0, &n) < 0)
//     return -1;
//   acquire(&tickslock);
//   ticks0 = ticks;
//   while(ticks - ticks0 < n){
//     if(myproc()->killed){
//       release(&tickslock);
//       return -1;
//     }
//     sleep(&ticks, &tickslock);
//   }
//   release(&tickslock);
//   return 0;
// }

// // return how many clock tick interrupts have occurred
// // since start.
// int
// sys_uptime(void)
// {
//   uint xticks;

//   acquire(&tickslock);
//   xticks = ticks;
//   release(&tickslock);
//   return xticks;
// }

// // my code starts here
// // extern struct proc proc[NPROC]; // Process table

// // int sys_gethistory(void) {
// //     struct proc *p;
// //     printf("PID\tProcess Name\tMemory\n");

// //     for (p = proc; p < &proc[NPROC]; p++) {
// //         if (p->state != UNUSED) {
// //             printf("%d\t%s\t%d\n", p->pid, p->name, p->sz);
// //         }
// //     }
// //     return 0;
// // }


// // my code starts here

// char command_history[MAX_HISTORY][MAX_COMMAND_LEN];
// int history_count = 0;



// int
// sys_block(void)
// {
//   int syscall_id;
//   if(argint(0, &syscall_id) < 0)
//     return -1;
  
//   // Get current process
//   struct proc *curproc = myproc();
  
//   // Block the syscall
//   curproc->blocked_syscalls[syscall_id] = 1;
//   cprintf("Blocked syscall %d\n", syscall_id);
  
//   return 0;
// }

// int
// sys_unblock(void)
// {
//   int syscall_id;
//   if(argint(0, &syscall_id) < 0)
//     return -1;
  
//   // Get current process
//   struct proc *curproc = myproc();
  
//   // Unblock the syscall
//   curproc->blocked_syscalls[syscall_id] = 0;
// return 0;
// }

// int sys_gethistory(void) {
//   struct history_entry *user_entries;
//   int user_max;
//   struct history_entry tmp_entries[MAX_HISTORY];
//   int i, count, entries_to_copy;

//   // Fetch user arguments
//   if(argptr(0, (void**)&user_entries, sizeof(struct history_entry)) < 0 ||
//      argint(1, &user_max) < 0) {
//     return -1;
//   }

//   if(user_max < 0) return -1;

//   acquire(&history_lock);
//   count = history_count;
//   if (count > MAX_HISTORY) count = MAX_HISTORY_ENTRIES;

//   // Copy kernel history to tmp_entries
//   for(i = 0; i < count; i++) tmp_entries[i] = history[i];

//   // Sort by start_time (ascending)
//   for(i = 0; i < count-1; i++) {
//     for(int j = 0; j < count-i-1; j++) {
//       if(tmp_entries[j].start_time > tmp_entries[j+1].start_time) {
//         struct history_entry temp = tmp_entries[j];
//         tmp_entries[j] = tmp_entries[j+1];
//         tmp_entries[j+1] = temp;
//       }
//     }
//   }

//   entries_to_copy = (user_max < count) ? user_max : count;

//   // Copy to user space
//   for(i = 0; i < entries_to_copy; i++) {
//     if(copyout(proc->pgdir, (uint)user_entries + i*sizeof(struct history_entry),
//                &tmp_entries[i], sizeof(struct history_entry)) < 0) {
//       release(&history_lock);
//       return -1;
//     }
//   }

//   release(&history_lock);
//   return entries_to_copy;
// }




// int
// sys_gethistory(void)
// {
//   int n;
//   char *buf;
  
//   if(argint(0, &n) < 0 || argptr(1, &buf, MAX_COMMAND_LEN) < 0)
//     return -1;
    
//   if(n < 0 || n >= history_count)
//     return -1;
    
//   strncpy(buf, command_history[n], MAX_COMMAND_LEN);
//   return 0;
// }

// // In sysproc.c
// int
// sys_addhistory(void)
// {
//   char *cmd;
  
//   if(argptr(0, &cmd, MAX_COMMAND_LEN) < 0)
//     return -1;
    
//   // Shift history if we've reached the maximum
//   if(history_count == MAX_HISTORY) {
//     for(int i = 0; i < MAX_HISTORY - 1; i++) {
//       strncpy(command_history[i], command_history[i+1], MAX_COMMAND_LEN);
//     }
//     history_count--;
//   }
  
//   // Add new command to history
//   strncpy(command_history[history_count], cmd, MAX_COMMAND_LEN);
//   history_count++;
  
//   return 0;
// }

