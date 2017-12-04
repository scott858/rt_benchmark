#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

volatile sig_atomic_t stop_flag = 0;
static timer_t timerid;
static int64_t get_tick_count_us();
static void handler(int sig, siginfo_t *si, void *uc);
#define CLOCKID	CLOCK_REALTIME
#define INTERRUPT_INTERVAL_ms	1

int main(int argc, char *argv[])
{
    /* start hardware timer */

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGRTMIN, &sa, NULL) == -1)
    {

        perror("sigaction");

    }

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    if(timer_create(CLOCKID, &sev, &timerid) == -1)
    {

        perror("timer_create");

    }

    int interval_ms = INTERRUPT_INTERVAL_ms;
    struct itimerspec timspec;

    bzero(&timspec, sizeof(timspec));
    int seconds = interval_ms / 1000;
    timspec.it_interval.tv_sec = seconds;
    int nanoseconds = (interval_ms % 1000) * 1000000;
    timspec.it_interval.tv_nsec = nanoseconds;
    timspec.it_value.tv_sec = seconds;
    timspec.it_value.tv_nsec = nanoseconds;

    if(timer_settime(timerid, 0, &timspec, NULL) == -1)
    {

        perror("timer_settime");

    }

    while(1){};

    return(0);
}

int64_t get_tick_count_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t) tv.tv_sec * 1000000) + (int64_t) (tv.tv_usec);
}

void my_sig_interrupt(int sig) {
    stop_flag = 1;
}

static void handler(int sig, siginfo_t *si, void *uc)
{
    static int64_t last_time_us = 0;
    int64_t this_time_us = 0;

    int expected_interval_us = 1000 * INTERRUPT_INTERVAL_ms;

    if(si->si_code == SI_TIMER)
    {

        this_time_us = get_tick_count_us();
        int64_t jitter = (this_time_us - last_time_us) - expected_interval_us;
        printf("jitter[us]: %lld\n", jitter);
        last_time_us = this_time_us;

    }

}

int a_timer(int argc, char *argv[])
{

    struct sched_param schedparm;
    memset(&schedparm, 0, sizeof(schedparm));
    schedparm.sched_priority = 1; // lowest rt priority
    sched_setscheduler(0, SCHED_FIFO, &schedparm);

    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    int milliseconds = 3;
    struct itimerspec timspec;

    bzero(&timspec, sizeof(timspec));
    int nanoseconds = (milliseconds % 1000) * 1000000;
    int seconds = milliseconds / 1000;
    timspec.it_interval.tv_sec = seconds;
    timspec.it_interval.tv_nsec = nanoseconds;
    timspec.it_value.tv_sec = seconds;
    timspec.it_value.tv_nsec = nanoseconds;

    int res = timerfd_settime(timerfd, 0, &timspec, 0);
    int64_t expirations = 0;
    int64_t last_time_us = 0;
    int64_t this_time_us = 0;
    while ((res = read(timerfd, &expirations, sizeof(expirations))) && !stop_flag) {
        if (res < 0) {
            perror("read error");
        } else if (res >= 1) {

        this_time_us = get_tick_count_us();

        int64_t jitter = this_time_us - last_time_us - milliseconds * 1000;
        printf("jitter[us]: %lld\n", jitter);
        last_time_us = this_time_us;

        }

    }
    return EXIT_SUCCESS;
}


