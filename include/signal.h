
#ifndef SIGNAL_H
#define SIGNAL_H

#include <sys/types.h>

union sigval
{
    int sival_int; /* Integer signal value. */
    void *sival_ptr; /* Pointer signal value. */
};

typedef struct
{
    int           si_signo  ; /* Signal number. */
    int           si_code   ; /* Signal code. */
    int           si_errno  ; /* If non-zero, an errno value associated with this signal, as described in <errno.h>. */
    pid_t         si_pid    ; /* Sending process ID. */
    uid_t         si_uid    ; /* Real user ID of sending process. */
    void         *si_addr   ; /* Address of faulting instruction. */
    int           si_status ; /* Exit value or signal. */
    long          si_band   ; /* Band event for SIGPOLL. */
    union sigval  si_value  ; /* Signal value. */
} siginfo_t;

#include_next <signal.h>

#define sa_sigaction sa_handler

typedef int pthread_attr_t; /* TODO: something about it in sys/types.h */

struct sigevent
{
    int sigev_notify; /* Notification type. */
    int sigev_signo;  /* Signal number. */
    union sigval sigev_value; /* Signal value. */
    void (*sigev_notify_function)(union sigval); /* Notification function. */
    pthread_attr_t *sigev_notify_attributes; /* Notification attributes. */ 
};

#define SIGEV_NONE 0 /* No asynchronous notification is delivered when the event of interest occurs. */
#define SIGEV_SIGNAL 1 /* A queued signal, with an application-defined value, is generated when the event of interest occurs. */
#define SIGEV_THREAD 2 /* A notification function is called to perform notification. */

#define SA_NOCLDSTOP   1 /* Do not generate SIGCHLD when children stop */
#define SA_ONSTACK     2 /* Causes signal delivery to occur on an alternate stack. */
#define SA_RESETHAND   4 /* Causes signal dispositions to be set to SIG_DFL on entry to signal handlers. */
#define SA_RESTART     8 /* Causes certain functions to become restartable. */
#define SA_SIGINFO    16 /* Causes extra information to be passed to signal handlers at the time of receipt of a signal. */
#define SA_NOCLDWAIT  32 /* Causes implementations not to create zombie processes or status information on child termination. See sigaction. */
#define SA_NODEFER    64 /* Causes signal not to be automatically blocked on entry to signal handler. */

int killpg (pid_t, int);
int sigaction (int, const struct sigaction *, struct sigaction *);
int sigaddset (sigset_t *, const int);
int sigdelset (sigset_t *, const int);
int sigismember (const sigset_t *, int);
int sigfillset (sigset_t *);
int sigemptyset (sigset_t *);
int sigpending (sigset_t *);
int sigsuspend (const sigset_t *);
int sigpause (int);

#endif /* SIGNAL_H */

