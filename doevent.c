#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/doevent.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/rwsem.h>
#include <asm/errno.h>
struct event initevent;
struct mutex globallock;
struct rw_semaphore  rwlock;

void __init doevent_init(void){
  initevent.eventid = 0;
  initevent.uid = current_uid().val;
  initevent.gid = current_gid().val;
  initevent.uidflag = 1;
  initevent.gidflag = 1;
  initevent.procnum = 0;
  initevent.sigflag = 0;
  INIT_LIST_HEAD(&(initevent.eventlist));
  init_waitqueue_head(&(initevent.waitp));
  mutex_init(&globallock);
  init_rwsem(&rwlock);
}

struct event * doevent_search(long eventid){
  struct event * target;
  list_for_each_entry(target, &(initevent.eventlist), eventlist){
    if(target->eventid == eventid){
      return target;
    }
  }
  return NULL;
}

long doevent_count(void){
  struct list_head * pos;
  long count = 0;
  list_for_each(pos, &(initevent.eventlist)){
    count++;
  }
  return count;
}

asmlinkage long sys_doeventopen(void) {
  struct event * newevent = kmalloc (sizeof(struct event),GFP_KERNEL);
  if (newevent == NULL){
    return -EFAULT;
  }
  INIT_LIST_HEAD(&(newevent->eventlist));
  newevent->uidflag = 0;
  newevent->gidflag = 0;
  newevent->procnum = 0;
  newevent->sigflag = 0;
  newevent->uid = current_uid().val;
  newevent->gid = current_gid().val;
  mutex_lock(&globallock);
  list_add_tail(&(newevent->eventlist), &(initevent.eventlist));
  newevent->eventid = (list_entry((newevent->eventlist).prev,
				  struct event, eventlist)->eventid) + 1;
  init_waitqueue_head(&(newevent->waitp));
  mutex_unlock(&globallock);
  return newevent->eventid;
}

asmlinkage long sys_doeventsig(int eventid){
  uid_t cur_uid = current_uid().val;
  gid_t cur_gid = current_gid().val;
  struct event * eventsig;
  long procnum;
  mutex_lock(&globallock);
  eventsig = doevent_search(eventid);
  if(eventsig == NULL){
    mutex_unlock(&globallock);
    return -1;
  }
  if(!(eventsig->uid == cur_uid && eventsig->uidflag)
     && !(eventsig->gid == cur_gid && eventsig->gidflag)
     && cur_uid != 0){
    mutex_unlock(&globallock);
    return -1;
  }
  procnum = eventsig->procnum;
  eventsig->procnum = 0;
  eventsig->sigflag = 1;
  wake_up_interruptible(&(eventsig->waitp));
  mutex_unlock(&globallock);
  return procnum;
}

asmlinkage long sys_doeventclose(int eventid){
  uid_t cur_uid = current_uid().val;
  gid_t cur_gid = current_gid().val;
  struct event * eventclose;
  long procnum = 0;
  //unsigned long flags;
  mutex_lock(&globallock);
  eventclose = doevent_search(eventid);
  if(eventclose == NULL){
    mutex_unlock(&globallock);
    return -1;
  }
  if(!(eventclose->uid == cur_uid && eventclose->uidflag)
     && !(eventclose->gid == cur_gid && eventclose->gidflag)
     && cur_uid != 0){
    mutex_unlock(&globallock);
    return -1;
  }
  procnum = eventclose->procnum;
  eventclose->sigflag = 1;
  wake_up_interruptible(&(eventclose->waitp));
  down_write(&rwlock);
  list_del(&(eventclose->eventlist));
  kfree(eventclose);
  up_write(&rwlock);
  mutex_unlock(&globallock);
  return procnum;
}

asmlinkage long sys_doeventwait(int eventid){
  uid_t cur_uid = current_uid().val;
  gid_t cur_gid = current_gid().val;
  struct event * eventwait;
  //unsigned long flags;
  mutex_lock(&globallock);
  eventwait = doevent_search(eventid);
  if(eventwait == NULL){
    mutex_unlock(&globallock);
    return -1;
  }
  if(!(eventwait->uid == cur_uid && eventwait->uidflag)
     && !(eventwait->gid == cur_gid && eventwait->gidflag)
     && cur_uid != 0){
    mutex_unlock(&globallock);
    return -1;
  }
  eventwait->procnum ++;
  eventwait->sigflag = 0;
  down_read(&rwlock);
  mutex_unlock(&globallock);
  wait_event_interruptible((eventwait->waitp), eventwait->sigflag);
  up_read(&rwlock);
  return 1;
}

asmlinkage long sys_doeventinfo(int num, int *eventids){
  int i = 0;
  int * eventidsk = kmalloc(num * sizeof(int), GFP_KERNEL);
  struct list_head * pos;
  struct event * eventret;
  long actnum = -1;
  if(eventidsk == NULL){
    return -EINVAL;
  }
  mutex_lock(&globallock);
  actnum = doevent_count();
  if (eventids == NULL){
    mutex_unlock(&globallock);
    return actnum;
  }
  if (actnum > num ){
    mutex_unlock(&globallock);
    return -1;
  }
  if(copy_from_user(eventidsk, eventids, num * sizeof(int)) != 0){
    mutex_unlock(&globallock);
    return -EINVAL;
  }
    
  list_for_each(pos, &(initevent.eventlist)){

    eventret = list_entry(pos, struct event, eventlist);
    eventidsk[i] = (int)(eventret->eventid);
    i++;
  }

  if(copy_to_user(eventids, eventidsk, num * sizeof(int)) !=  0){
    mutex_unlock(&globallock);
    return -EINVAL;
  }

  mutex_unlock(&globallock);
  return actnum;
}

asmlinkage long sys_doeventchown(int eventid, uid_t uid, gid_t gid){
  struct event * eventchown;
  uid_t cur_uid;
  mutex_lock(&globallock);
  eventchown = doevent_search(eventid);
  cur_uid = eventchown->uid;
  if(cur_uid != 0 && cur_uid != uid){
    mutex_unlock(&globallock);
    return -1;
  }
  if(eventchown == NULL){
    mutex_unlock(&globallock);
    return -1;
  }
  eventchown->uid = uid;
  eventchown->gid = gid;
  mutex_unlock(&globallock);
  return 1;
}


long sys_doeventchmod(int eventID, int UIDFlag, int GIDFlag){
  struct event * eventchmod;
  uid_t event_uid;
  mutex_lock(&globallock);
  eventchmod = doevent_search(eventID);
  if(eventchmod == NULL){
    mutex_unlock(&globallock);
    return -1;
  }

  event_uid = eventchmod->uid;
  if(current_uid().val != 0 && current_uid().val != event_uid){
    mutex_unlock(&globallock);
    return -1;
  }

  eventchmod->uidflag = UIDFlag;
  eventchmod->gidflag = GIDFlag;

  mutex_unlock(&globallock);
  return 1;
}

long sys_doeventstat(int eventID, uid_t * UID, gid_t * GID, int * UIDFlag, int * GIDFlag){

  uid_t k_UID;
  gid_t k_GID;
  int k_UIDFlag;
  int k_GIDFlag;
  struct event * event_stat;
  if(UID == NULL || GID == NULL || UIDFlag == NULL || GIDFlag == NULL){
    return -EINVAL;
  }
  if(copy_from_user(&k_UID, UID, sizeof(uid_t)) != 0 || 
     copy_from_user(&k_GID, GID, sizeof(gid_t)) != 0 ||
     copy_from_user(&k_UIDFlag, UIDFlag, sizeof(int)) != 0 ||
     copy_from_user(&k_GIDFlag, GIDFlag, sizeof(int)) != 0)
    return -EFAULT;
  
  mutex_lock(&globallock);
  event_stat = doevent_search(eventID);
  if(event_stat == NULL){
    mutex_unlock(&globallock);
    return -1;
  }
  k_UID = event_stat->uid;
  k_GID = event_stat->gid;
  k_UIDFlag = event_stat->uidflag;
  k_GIDFlag = event_stat->gidflag;

  if(copy_to_user(UID, &k_UID, sizeof(uid_t))!=0
     ||copy_to_user(GID, &k_GID, sizeof(uid_t))!=0
     ||copy_to_user(UIDFlag, &k_UIDFlag, sizeof(int))!=0
     ||copy_to_user(GIDFlag, &k_GIDFlag, sizeof(int))!=0)
  {
    mutex_unlock(&globallock);
    return -EFAULT;
  }
  mutex_unlock(&globallock); 
  return 1; 
}
