/* 
 * This file implements a new systemcall.
 * It collects the infomation of struct prinfo defined in "include/linux/prinfo.h".
 * */

#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/prinfo1.h>
#include <linux/syscalls.h>
#include <linux/sched.h>   
#include <linux/cred.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <asm/errno.h>

#define MAXPROGLENG 15

/* new syscall */
SYSCALL_DEFINE1(prinfo1, struct prinfo *, info) {
    struct task_struct *target_task;  // for target process
    struct list_head *position;       // to go through children and sibling lists
    struct task_struct *child_process;
    struct task_struct *sibling_process;
    struct prinfo *info_in_kernel;
    unsigned long youngest_child_born_time = 0;
    unsigned long younger_sibling_born_time = ULONG_MAX;
    unsigned long older_sibling_born_time = 0;
    struct fdtable *files_table;
    int i = 0;

    /* printk("Hello from new sys_call\n"); */
    
    if ((info_in_kernel = ((struct prinfo *)kmalloc(sizeof(struct prinfo), GFP_KERNEL))) == NULL) {
        /* printk("kmallc() failed\n"); */
        return ENOMEM;
    }

    if (copy_from_user(info_in_kernel, info, sizeof(struct prinfo)) != 0) {
        /* copy_from_user failed */
        /* printk("copy_from_user() failed \n"); */
        return EFAULT;
    }

    target_task = find_task_by_vpid(info_in_kernel->pid);
    if (target_task == NULL) {
        /* unidentified pid */
        /* printk("pid %d does no exist!\n", info_in_kernel->pid); */
        return EINVAL;
    } 

    info_in_kernel->state = target_task->state;
    info_in_kernel->pid = target_task->pid;
    info_in_kernel->parent_pid = target_task->real_parent->pid;

    /* find the youngest child of the target_task process */
    info_in_kernel->youngest_child_pid = -1;   // -1 means the target_task porcess doesn't have a child
    info_in_kernel->cutime = 0;
    info_in_kernel->cstime = 0;
    list_for_each(position, &target_task->children) {
        child_process = list_entry(position, struct task_struct, sibling);
        if (child_process->start_time > youngest_child_born_time) {
            /* find a younger child */
            youngest_child_born_time = child_process->start_time;
            info_in_kernel->youngest_child_pid = child_process->pid;
        }

        // accumulate children's running time
        info_in_kernel->cutime = info_in_kernel->cutime + child_process->utime;   
        info_in_kernel->cstime = info_in_kernel->cstime + child_process->stime;
    }

    /* find the younger and older sibling of the target_task process */
    info_in_kernel->younger_sibling_pid = -1;  // -1 means the target_task process has no younger sibling
    info_in_kernel->older_sibling_pid = -1;   
    list_for_each(position, &(target_task->real_parent->children)) {
        sibling_process = list_entry(position, struct task_struct, sibling);
        if (sibling_process->start_time < target_task->start_time && sibling_process->start_time > older_sibling_born_time) {
            older_sibling_born_time = sibling_process->start_time;
            info_in_kernel->older_sibling_pid = sibling_process->pid;
        }
        if (sibling_process->start_time > target_task->start_time && sibling_process->start_time < younger_sibling_born_time) {
            younger_sibling_born_time = sibling_process->start_time;
            info_in_kernel->younger_sibling_pid = sibling_process->pid;
        }
    }

    info_in_kernel->start_time = target_task->real_start_time;

    info_in_kernel->user_time = target_task->utime;
    info_in_kernel->sys_time = target_task->stime;

    info_in_kernel->uid = target_task->real_cred->uid.val;
    strncpy(info_in_kernel->comm, target_task->comm, MAXPROGLENG);

    /* get the pending signals */
    info_in_kernel->signal = (target_task->signal->shared_pending.signal.sig)[0];

    /* get file related info */
    files_table = files_fdtable(target_task->files);

    while(files_table->fd[i] != NULL) i++;

    info_in_kernel->num_open_fds=i;
    
    /* before leaving, copy from kernel to user */
    if (copy_to_user(info, info_in_kernel, sizeof(struct prinfo)) != 0) {
        /* printk("copy_to_user() failed\n"); */
        return EFAULT;
    }

    /* return 0 if executed successfully */
    return 0;
}

