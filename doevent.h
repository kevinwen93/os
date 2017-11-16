#ifndef __DOEVENT_H
#define __DOEVENT_H

#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/cred.h>
#include <linux/rwsem.h>
#include <linux/sched.h>

struct event{
  unsigned long eventid;
  uid_t uid;
  gid_t gid;
  int uidflag;
  int gidflag;
  struct list_head eventlist;
  wait_queue_head_t waitp;
  unsigned long procnum;
  int sigflag;
};

void __init doevent_init(void);
struct event * doevent_search(long eventid);
long doevent_count(void);

asmlinkage long doeventopen(void);
asmlinkage long doeventclose(int eventid);
asmlinkage long doeventwait(int eventid);
asmlinkage long doeventsig(int eventid);
asmlinkage long doeventinfo(int num, int *eventids);
asmlinkage long doeventchown(int eventid, uid_t uid, gid_t gid);
asmlinkage long doeventchmod(int eventid, int uidflag, int gidflag);
asmlinkage long doeventstat(int eventid, uid_t *uid, gid_t *gid,
			    int * uidflag, int * gidflag);

extern struct event initevent;
extern struct mutex globallock;
extern struct rw_semaphore rwlock;

#endif
