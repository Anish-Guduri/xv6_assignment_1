// history.h
#ifndef HISTORY_H
#define HISTORY_H

#define MAX_HISTORY 64

struct history_entry {
  int pid;
  char name[16];
  uint sz;
  uint ctime;
};

#endif