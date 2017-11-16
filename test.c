#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <errno.h>

/*
 * Struct definition:
 * 	Define the struct prinfo which is the data structure we desired to contain the information of process
 */
struct prinfo {
	long state; 					/*current state of process */
	pid_t pid; 						/*process id (input) */
	pid_t parent_pid;				/*process id of parent */
	pid_t youngest_child_pid; 		/*process id of youngest child */
	pid_t younger_sibling_pid; 		/*pid of the oldest among younger siblings */
	pid_t older_sibling_pid; 		/*pid of the youngest among older siblings */
	unsigned long start_time;		/*process start time */
	unsigned long user_time;		/*CPU time spent in user mode */
	unsigned long sys_time;			/*CPU time spent in system mode */
	unsigned long cutime;			/*total user time of children */
	unsigned long cstime;			/*total system time of children */
	long uid;						/*user id of process owner */
	char comm[16];					/*name of program executed */
	unsigned long signal;			/*The set of pending signals */
	unsigned long num_open_fds;		/*Number of open file descriptors */
};

/*
 * Function: main()
 *
 * Description:
 *   Entry point for this program.
 *
 * Inputs:
 *   argc - The number of argument with which this program was executed.
 *   argv - Array of pointers to strings containing the command-line arguments. 
 *
 * Return value:
 *   0 - This program terminated normally.
 */
int main (int argc, char ** argv) {

	struct prinfo p;
	struct prinfo p1;

	/* check the first argv. If null, notify and return.*/
	if (argc < 2)
	{
		perror("test supposes to get an pid as input \n");
		return 1;
	}
	else{
		p.pid = strtol(argv[1],NULL,10);
		p1.pid =  strtol(argv[1],NULL,10);
	}
	
	/* Call our new system call */
	if (syscall (181, &p)){
		perror("syscall happens to error\n");
		return 1;
	}

	if (syscall (322, &p1)){
		perror("syscall happens to error\n");
		return 1;
	}
	
	/* Print the information of process with input pid */
	printf("process id: %d\n", p.pid);
	printf("state: %d\n", p.state);
	if (p.parent_pid == -1) printf("ppid: no parent \n");
		else printf("ppid: %d\n", p.parent_pid);
	if (p.youngest_child_pid == -1) printf("youngtest_cpid: no youngest child\n");
		else printf("youngest_cpid: %d\n", p.youngest_child_pid);
	if (p.younger_sibling_pid == -1) printf("younger_spid: no such sibling\n");
		else printf("younger_spid: %d\n", p.younger_sibling_pid);
	if (p.older_sibling_pid == -1) printf("older_spid: no such sibling\n");
		else printf("older_spid: %d\n", p.older_sibling_pid);
	printf("start time: %lu\n",p.start_time);
	printf("CPU time in user mode: %lu\n", p.user_time);
	printf("CPU time in system mode: %lu\n", p.sys_time);
	printf("total user time of children: %lu\n", p.cutime);
	printf("total system time of children: %lu\n", p.cstime);
	printf("uid: %d \n", p.uid);
	printf("comm: %s\n", p.comm);
	printf("pending signal: %lu\n", p.signal);
	printf("num of open fds: %lu\n", p.num_open_fds);

	/* Exit the program */

	printf("process id: %d\n", p1.pid);
	printf("state: %d\n", p1.state);
	if (p1.parent_pid == -1) printf("ppid: no parent \n");
		else printf("ppid: %d\n", p1.parent_pid);
	if (p1.youngest_child_pid == -1) printf("youngtest_cpid: no youngest child\n");
		else printf("youngest_cpid: %d\n", p1.youngest_child_pid);
	if (p1.younger_sibling_pid == -1) printf("younger_spid: no such sibling\n");
		else printf("younger_spid: %d\n", p1.younger_sibling_pid);
	if (p1.older_sibling_pid == -1) printf("older_spid: no such sibling\n");
		else printf("older_spid: %d\n", p1.older_sibling_pid);
	printf("start time: %lu\n",p1.start_time);
	printf("CPU time in user mode: %lu\n", p1.user_time);
	printf("CPU time in system mode: %lu\n", p1.sys_time);
	printf("total user time of children: %lu\n", p1.cutime);
	printf("total system time of children: %lu\n", p1.cstime);
	printf("uid: %d \n", p1.uid);
	printf("comm: %s\n", p1.comm);
	printf("pending signal: %lu\n", p1.signal);
	printf("num of open fds: %lu\n", p1.num_open_fds);
	return 0;
}
