#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

struct  child {
  tid_t tid ;
  bool exited;
  int exit_status;
  struct  list_elem  tid_elem ;
};
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
void split (const char *s,char** argv);
#endif /* userprog/process.h */
