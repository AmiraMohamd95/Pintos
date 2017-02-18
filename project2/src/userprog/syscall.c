#include "userprog/syscall.h"
#include <stdio.h>
#include <list.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "pagedir.h"
#include "devices/shutdown.h"
#include "devices/input.h"
struct file_attr
{
  int fd;
  struct file *f;
  struct list_elem  elem;
};

bool validate (void * x){

  if (x == NULL)
       return false ;
    if (!is_user_vaddr(x)){
          return false ;
    }
    void *p = pagedir_get_page (thread_current()->pagedir, x);
    if (p  ==   NULL ){
	   return false ;
   }
  return true ;
}
struct file_attr* get_file (int fd){
	struct file_attr* fileAttr ;
	struct list_elem* e;
	for (e = list_begin (&thread_current()->opened_files); e != list_end (&thread_current()->opened_files); e = list_next (e)){
		fileAttr = list_entry (e, struct file_attr,elem );
		if(fileAttr->fd == fd) {
  		return fileAttr;
    }
	}
	return NULL;
}

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
static void
syscall_handler (struct intr_frame *f)
{

    void* p =f->esp ;

    if (!validate (p)){
      exit(-1);
   }
   switch (*(int *) p) {
    case SYS_HALT:// no parameters and void
    {
      halt();
      break ;
    }
    case SYS_EXIT: // 1 parameter  and void
    {
      void* a = f->esp + 4 ;
      if (!validate (a))
         exit(-1);
      int status= *(int *)a;
      exit(status);
    break;
    }
    case SYS_EXEC: // it has one parameter .
    {
      void  * a = f->esp +4 ;
      if (!validate (a ))
         exit(-1);
      char * s = *(char **)a ;
      f->eax = exec(s);
      break ;
    }
    case SYS_WAIT: // it has one parameter .
    {
      void  * a = f->esp +4 ;
      if (!validate (a ))
         exit(-1);
      int pid = *(int *)a ;
  		f ->eax = wait(pid);
      break ;
    }
   case SYS_CREATE: // it has two parameter .
    {
      const char *file  ;
      unsigned initial_size ;
      void  * a = f->esp +4 ;
      if (!validate (a))
        exit(-1);
      if (!validate(*(char **)a )){
        exit(-1);
      }
      file = *(char **)a ;
      void  * a2 = a + 4 ;
      initial_size = *(unsigned *)a2 ;
      if (!validate (a2))
         exit(-1);
      f ->eax = create (file,initial_size);
      break;
    }
    case SYS_REMOVE: // it has one parameter .
     {
      const char *file ;
       void  * a = f->esp +4 ;
       if (!validate (a))
             exit(-1);
       if (!validate(*(char **)a )){
         exit(-1);
       }
       file = *(char **)a ;
       f ->eax = remove(file);
       break ;
     }
     case SYS_OPEN: // it has one parameter .
      {
      const char *file ;
        void  * a = f->esp +4 ;
        if (!validate (a))
              exit(-1);
        if (!validate(*(char **)a )){
          exit(-1);
        }
        file = *(char **)a ;
        f ->eax = open(file);
        break ;
      }
      case SYS_FILESIZE: // it has one parameter .
       {

         void  * a = f->esp +4 ;
         if (!validate (a ))
             exit(-1);
         int fd = *(int *)a ;
         f ->eax = filesize(fd);
        break ;
       }

       case SYS_READ: // it has 3 parameter .
        {
          int fd; void *buffer; unsigned length ;
          void *arg0 = p + 4;
          if (!validate ((int * )arg0 ))
             exit(-1);
          fd = *(int *)arg0 ;
          void *arg1 = p+ 8;
          if (!validate ((char **)arg1))
                exit(-1);
          if (!validate(*(char **)arg1)){
            exit(-1);
          }
          buffer = *(char **)arg1;
          void *arg2 = p + 12;
          if (!validate ((unsigned *)arg2 ))
                exit(-1);
          length= *(unsigned *)arg2 ;
          f ->eax = read(fd,buffer,length);
        break ;
        }
        case SYS_WRITE: // it has 3 parameter .
         {
					 int fd; char *buffer; unsigned length ;
           void *arg0 = p + 4;
           if (!validate ((int *)arg0 ))
              exit(-1);
           fd = *(int *)arg0 ;
          void *arg1 = p + 8;
          if (!validate ((char **)arg1))
              exit(-1);
          if (!validate(*(char **)arg1)){
            exit(-1);
          }
           buffer= *(char **)arg1 ;
           void *arg2 = p + 12;
           if (!validate ((unsigned *)arg2 ))
                 exit(-1);
           length= *(unsigned *)arg2 ;
           f ->eax = write(fd,buffer,length);
           break;
         }
         case SYS_SEEK: // it has two parameter and void
          {
						int fd; unsigned position;
						if (!validate (f->esp+4))
							   exit(-1);
            void  * a = f->esp +4;
            fd = *(int *)a ;
            a = a+4;
            position= *(unsigned *)a ;
            seek (fd,position);
            break ;
          }
          case SYS_TELL: // it has one parameter .
          {
						if (!validate (f->esp+4))
					   exit(-1);
            void  * a = f->esp +4;
            int  fd = *(int *)a ;
        		f ->eax = tell(fd);
            break ;
          }
          case SYS_CLOSE: // it has one parameter  and void
						{
              void  * a = f->esp +4 ;
              if (!validate (a))
                    exit(-1);
						  int  fd = *(int *)a ;
             close (fd);
             break ;
          }
          default :
          {
            exit(-1);
          }
      }
}
void halt (void)
{
  shutdown_power_off();
}

void exit (int status)
{
	printf ("%s: exit(%d)\n",thread_current()->name,status);
  struct child *c;
  struct list_elem * e;
 for ( e= list_begin (&thread_current()->parent->children); e != list_end (&thread_current()->parent->children); e = list_next (e)){
     c = list_entry (e, struct child, tid_elem);
     if (c->tid == thread_current()->tid)
     {
       list_remove(&c->tid_elem);
       c->exited = true ;
       c->exit_status = status ;
       c->tid = thread_current()->tid ;
       list_push_back( &thread_current()->parent->children,&c->tid_elem);

     }
   }
	thread_exit ();
}
int wait (pid_t pid)
{
	return process_wait(pid);
}

bool create (const char *file, unsigned initial_size)
{
	return filesys_create(file,initial_size);
}
bool remove (const char *file)
{
	 return filesys_remove (file);
}
int open (const char *file)
{

	 struct file* openedFile = filesys_open (file);
	 if (openedFile == NULL)
	 	return -1;

	struct file_attr* fileAttr = malloc (sizeof(struct file_attr));
	// we couldn't allocate memory
	if (fileAttr == NULL){
    file_close (openedFile);
    return -1;
  }

  fileAttr -> fd = thread_current()->next_file_id ;
  thread_current()->next_file_id  =   thread_current()->next_file_id +1 ;
	fileAttr -> f = openedFile ;
	list_push_back (&thread_current()->opened_files, &fileAttr->elem);
	 return fileAttr->fd;
}
/* Returns the size, in bytes, of the file open as fd. */
int filesize (int fd)
{
	struct file_attr* fileAttr = get_file(fd);
	if (fileAttr != NULL){
		return file_length (fileAttr->f);
	}
	return -1;
}

int read (int fd, void *buffer, unsigned size)
{
  struct file_attr *fa  = get_file(fd); // mapping
  if (fd == STDIN_FILENO){
    return input_getc() ;
}
else if (fa  != NULL){
    return file_read(fa->f, buffer, size);
  }
  return -1;
}

int write (int fd, const void *buffer, unsigned size)
{
  if (fd == STDOUT_FILENO){
    putbuf((char *)buffer, size);
    return (int)size;
  }else if (get_file (fd) != NULL){

    return (int)file_write(get_file(fd)->f, buffer, size);
  }

  return -1;
}

void seek (int fd, unsigned position) //Changes the next byte to be read or written in open file fd to position
{
  struct file_attr *fa  =get_file(fd); // mapping
  if (fa  != NULL){
     file_seek (fa->f , position);
  }
}

unsigned tell (int fd)  // Returns the position of the next byte to be read or written
{
  struct file_attr *fa  =get_file(fd); // mapping
  if (fa  != NULL){
    return file_tell  (fa->f);
  }
  return -1;
}
pid_t exec (const char *cmd_line){
  return  process_execute(cmd_line);
}

void close (int fd)
{
	struct file_attr* fileAttr = get_file(fd);
	if (fileAttr != NULL){
    lock_acquire (&thread_current()->wall);
		file_close(fileAttr->f);
		list_remove(&fileAttr->elem);
    free(fileAttr);
    lock_release (&thread_current()->wall);
	}
}
void close_all (void)
{
  lock_init(&thread_current()->wall);
  while (!list_empty( &thread_current()->opened_files)){
    struct file_attr* fileAttr = list_entry (list_pop_front( &thread_current()->opened_files), struct file_attr, elem);
  	file_close(fileAttr->f);
  	list_remove(&fileAttr->elem);
    free(fileAttr);
  }
  while (!list_empty( &thread_current()->children)){
      struct child*  c = list_entry (list_pop_front( &thread_current()->children), struct child, tid_elem);
      list_remove(&c->tid_elem);
      free(c);
    }
}
