#include <sys/fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>
#include <common.h>

/* Cloning flags.  */
# define CSIGNAL       0x000000ff /* Signal mask to be sent at exit.  */
# define CLONE_VM      0x00000100 /* Set if VM shared between processes.  */
# define CLONE_FS      0x00000200 /* Set if fs info shared between processes.  */
# define CLONE_FILES   0x00000400 /* Set if open files shared between processes.  */
# define CLONE_SIGHAND 0x00000800 /* Set if signal handlers shared.  */
# define CLONE_PTRACE  0x00002000 /* Set if tracing continues on the child.  */
# define CLONE_VFORK   0x00004000 /* Set if the parent wants the child to
                     wake it up on mm_release.  */
# define CLONE_PARENT  0x00008000 /* Set if we want to have the same
                     parent as the cloner.  */
# define CLONE_THREAD  0x00010000 /* Set to add to same thread group.  */
# define CLONE_NEWNS   0x00020000 /* Set to create new namespace.  */
# define CLONE_SYSVSEM 0x00040000 /* Set to shared SVID SEM_UNDO semantics.  */
# define CLONE_SETTLS  0x00080000 /* Set TLS info.  */
# define CLONE_PARENT_SETTID 0x00100000 /* Store TID in userlevel buffer
                       before MM copy.  */
# define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory
                        location to clear.  */
# define CLONE_DETACHED 0x00400000 /* Create clone detached.  */
# define CLONE_UNTRACED 0x00800000 /* Set if the tracing process can't
                      force CLONE_PTRACE on this clone.  */
# define CLONE_CHILD_SETTID 0x01000000 /* Store TID in userlevel buffer in
                      the child.  */
# define CLONE_STOPPED 0x02000000 /* Start in stopped state.  */
# define CLONE_NEWUTS   0x04000000  /* New utsname group.  */
# define CLONE_NEWIPC   0x08000000  /* New ipcs.  */
# define CLONE_NEWUSER  0x10000000  /* New user namespace.  */
# define CLONE_NEWPID   0x20000000  /* New pid namespace.  */
# define CLONE_NEWNET   0x40000000  /* New network namespace.  */
# define CLONE_IO   0x80000000  /* Clone I/O context.  */

#define COUNT MICROBENCH_REPEAT

extern int __my_clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...);

typedef void (*start_func)(void);

static unsigned start;
static unsigned times[COUNT];
static size_t c = 0;
static unsigned stack[1024];

static void *thread_func(void *arg) {
    unsigned end = get_cycles();
    times[c++] = end - start;
    /* to be able to call other library routines like printf here we would need a real pthread to
     * get attributes like the stacksize. */
    return arg;
}

static void start_clone() {
    start = get_cycles();
    int pid = __my_clone((int (*)(void*))thread_func, stack + 1024,
        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_SYSVSEM,
        NULL);
    if(pid == -1)
        perror("clone");
    if(waitpid(pid, NULL, __WCLONE) == -1)
        perror("waitpid");
}

static void start_pthread() {
    pthread_t tid;
    start = get_cycles();
    if(pthread_create(&tid, NULL, thread_func, NULL) == -1)
        perror("pthread_create");
    pthread_join(tid, NULL);
}

static void do_bench(const char *name, start_func func) {
    c = 0;
    size_t i;
    for(i = 0; i < COUNT; ++i)
        func();

    unsigned average = avg(times, COUNT);
    printf("[thread] Cycles per %s (avg): %u (%u)\n", name, average, stddev(times, COUNT, average));
}

int main() {
    do_bench("pthread_start", start_pthread);
    do_bench("clone", start_clone);
    return 0;
}
