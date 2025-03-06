#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
// #include <string.h>

// #include "exec.c"
// #include "history.h"
// #include "sh.c"


// #define MAX_HISTORY_ENTRIES 64
// static struct history_entry history[MAX_HISTORY_ENTRIES];
// static int history_count = 0;
// static struct spinlock history_lock;


#define MAX_HISTORY 16

struct history_entry {
    int pid;
    char name[16];
    uint memory_usage;
};

struct history {
    struct history_entry entries[MAX_HISTORY];
    int count;
};

// Define the global history object (this line must be active)
// struct history cmd_history = { .count = 0 };
struct history cmd_history = { .count = 0 };
// Declare cmd_history as extern so it can be accessed in multiple files
// extern struct history cmd_history;

// struct history cmd_history = { .count = 0 }; 

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
    // Calculate total memory usage
    uint memory_usage = curproc->sz;

    // // Update history (store latest commands in circular manner)
    // if (cmd_history.count < MAX_HISTORY) {
    //     cmd_history.count++;
    // }
    // for (int i = MAX_HISTORY - 1; i > 0; i--) {
    //     cmd_history.entries[i] = cmd_history.entries[i - 1];
    // }
    // cmd_history.entries[0].pid = curproc->pid;
    // safestrcpy(cmd_history.entries[0].name, path, sizeof(cmd_history.entries[0].name));
    // cmd_history.entries[0].memory_usage = memory_usage;
    if ((path[0] == 's' && path[1] == 'h' && path[2] == '\0') ||
    (path[0] == '/' && path[1] == 'i' && path[2] == 'n' && path[3] == 'i' && path[4] == 't') && path[5] == '\0') { 
    // Skip updating history.
} else {
    // Update history.

    if (cmd_history.count < MAX_HISTORY) {
      int i = cmd_history.count;  // Append at the end
      cmd_history.entries[i].pid = curproc->pid;
      safestrcpy(cmd_history.entries[i].name, path, sizeof(cmd_history.entries[i].name));
      // safestrcpy(cmd_history.entries[i].name, last, sizeof(cmd_history.entries[i].name));
      cmd_history.entries[i].memory_usage = memory_usage;
      cmd_history.count++;
  } else {
      // History is full. Shift entries left (discard the oldest)
      for (int i = 0; i < MAX_HISTORY - 1; i++) {
          cmd_history.entries[i] = cmd_history.entries[i+1];
      }
      int i = MAX_HISTORY - 1;
      cmd_history.entries[i].pid = curproc->pid;
      safestrcpy(cmd_history.entries[i].name, path, sizeof(cmd_history.entries[i].name));
      cmd_history.entries[i].memory_usage = memory_usage;
  }
}




  ilock(ip);
  if (!(ip->mode & 4))
  {
    iunlockput(ip);
    // cprintf("ip value %d\n", ip->minor);
    cprintf("Operation execute failed\n");
    end_op();

return -1;

}

  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;

//   acquire(&history_lock);
// if (history_count < MAX_HISTORY_ENTRIES) {
//   struct history_entry *entry = &history[history_count];
//   entry->pid = curproc->pid;
//   safestrcpy(entry->name, curproc->name, sizeof(entry->name));
//   entry->total_memory = curproc->sz + 4096; // sz + stack
//   entry->start_time = curproc->start_time;
//   history_count++;
// }
// release(&history_lock);


  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
