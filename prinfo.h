/* #include <types.h> */

struct prinfo {
    long state;  /* current state of process */
    pid_t pid;   /* process id (input) */
    pid_t parent_pid;   
    pid_t youngest_child_pid;    /* pid of the youngest child */
    pid_t younger_sibling_pid;   /* pid of the younger sibling  */
    pid_t older_sibling_pid;     /* pid of the youngest among older siblings */
    unsigned long start_time;    /* porcess start time */
    unsigned long user_time;     /* CPU time spent in user mode */
    unsigned long sys_time;      /* CPU time spent in system mode */
    unsigned long cutime;        /* total user time of children */
    unsigned long cstime;        /* total system time of child */
    long uid;                    /* user id of process owner */
    char comm[16];               /* name of program executed */
    unsigned long signal;        /* the set of pending signals */
    unsigned long num_open_fds;  /* number of open file descriptors */
};
