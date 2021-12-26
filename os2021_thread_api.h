#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 20000
#define eventNum 7

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "function_libary.h"

typedef struct thread
{
    char* name;
    char* entryFunction;
    char* base_priority;
    char* priority;
    int cancelMode;
    int tid;
    char* state; // READY, WAITING, RUNNING
    int isCancel;
    int waitTime;
    int q_time;
    int w_time;
    bool isWaiting;
    int isWaitingEvent;
    int waitingEvent;
    int waitTimeCount;
    int time_quantum;
    int isScheduled; // set to 1 when waiting queue -> ready queue

    struct thread *next;
} thread;

typedef struct queue
{
    struct thread *front, *rear;
} queue;


int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();

/* ready queue */
struct queue *ready_queue[3]; // H, M, L

/* waiting queue */
struct queue* waiting_queue[8][3]; // 8 events, each of them have 3 priorities

/* running_thread */
struct thread* running_thread;

/* queue func */
void enqueue(struct queue *q, struct thread *t);
void dequeue(struct queue *q);

void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Dispatcher();
void StartSchedulingSimulation();

void set_dispatch();
int check_running();

int context_num;

ucontext_t dispatch_context;




#endif
