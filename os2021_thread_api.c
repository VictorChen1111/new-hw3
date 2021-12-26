#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t timer_context;
ucontext_t contexts[5];

int threadNum = 1;
char *func_string = NULL;
context_num = -1;


int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    printf("%s is created.\n", job_name);

    /* return if not in funcs */
    if (
        strcmp(p_function, "Function1") &&
        strcmp(p_function, "Function2") &&
        strcmp(p_function, "Function3") &&
        strcmp(p_function, "Function4") &&
        strcmp(p_function, "Function5") &&
        strcmp(p_function, "ResourceReclaim")
    )return -1;
    // printf("Create %s\n",p_function);
    struct thread *t = (struct thread*)malloc(sizeof(struct thread));

    /* init thread */
    t->name = job_name;
    t->entryFunction = p_function;
    t->priority = priority;
    t->base_priority = priority;
    t->cancelMode = cancel_mode;
    t->tid = threadNum;
    t->state = "READY";
    t->isCancel = 0;
    t->q_time = 0;
    t->w_time = 0;
    t->time_quantum = 0;
    t->isScheduled = 0;
    t->isCancel = 0;
    t->waitingEvent= 0;
    t->isWaiting = 0;
    t->waitTime = 0;
    t->waitTimeCount = 0;

    if (!strcmp(priority, "H"))
    {
        enqueue(ready_queue[0], t);
        threadNum++;
    }
    else if (!strcmp(priority, "M"))
    {
        enqueue(ready_queue[1], t);
        // printf("%s is in the M queue.\n", ready_queue[1]->rear->name);
        // printf("the thread is in the M ready queue.\n");
        // printf("the rear name of the ready queue is %s.\n", ready_queue[1]->rear->name);
        threadNum++;
    }
    else if (!strcmp(priority, "L"))
    {
        enqueue(ready_queue[2], t);
        threadNum++;
    }
    else
    {
        printf("Error in creating thread.\n");
    }
    // printf("[TEST2]\n");
    /* return tid */
    return threadNum;
}

void OS2021_ThreadCancel(char *job_name)
{
    struct thread *temp = NULL;
    bool found = false;

    // Search for the ready queue.
    for(int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if (!strcmp(job_name, temp->name))
            {
                found = 1;
                break;
            }
            temp = temp->next;
        }

        if (found)
        {
            break;
        }
    }

    // Search for the waiting queue.
    if(!found)
    {
        for(int j = 0; j < 8; j++)
        {
            for(int i = 0; i < 3; i++)
            {
                if (waiting_queue[j][i]->front == NULL) continue;
                temp = waiting_queue[j][i]->front;
                while(temp)
                {
                    if (!strcmp(job_name, temp->name))
                    {
                        found = 1;
                        break;
                    }
                    temp = temp->next;
                }

                if (found)
                {
                    break;
                }
            }

            if (found)
            {
                break;
            }
        }
    }

    // Reclaimer cannot be terminated.
    if (!strcmp(temp->name, "reclaimer")) 
    {
        printf("found reclamier is ready to cacel.\n");
        return;
    }

    // If the canel_mode is 0, reclaimed by reclaimer.
    if (!temp->cancelMode)
    {
        temp->state = "TERMINATED";
    }
    // Cancel_mode is 1, inform the thread that another thread wants to cancel it.
    else
    {
        // printf("%s is ready to be cancel.\n", temp->name);
        temp->isCancel = 1;
        // printf("%s is ready to cancel.\n", temp->name);
        
    }
}


void OS2021_ThreadWaitEvent(int event_id)
{
    struct thread *temp = running_thread;
    // the thread calling this API must print the event it wants to wait
    // for on the terminal
    printf("%s wants to wait for event %d.\n", temp->name, event_id);

    // The running changes its state to WAITING and enters the event
    temp->state = "WAITING";
    temp->isWaitingEvent = 1;
    temp->waitingEvent = event_id;

    
    // waiting queue corresponding to event_id
    if (!strcmp(temp->priority, "H"))
    {
        // printf("[dequeue] => %s\n", ready_queue[0]->front->name);
        dequeue(ready_queue[0]);
        enqueue(waiting_queue[event_id][0], temp);
        temp->priority = "H";

        // printf("[enqueue] => %s\n", waiting_queue[event_id][0]->rear->name);
    }
    else if (!strcmp(temp->priority, "M"))
    {
        // printf("[dequeue] => %s\n", ready_queue[1]->front->name);

        dequeue(ready_queue[1]);
        
        enqueue(waiting_queue[event_id][0], temp);
        temp->priority = "H";
        // printf("[enqueue] => %s\n", waiting_queue[event_id][0]->rear->name);
        printf("The priority of thread %s is changed from M to H.\n", temp->name);
    }
    else if (!strcmp(temp->priority, "L"))
    {
        // printf("[dequeue] => %s\n", ready_queue[2]->front->name);

        dequeue(ready_queue[2]);
        enqueue(waiting_queue[event_id][1], temp);
        temp->priority = "M";

        // printf("[enqueue] => %s\n", waiting_queue[event_id][1]->rear->name);
        
        printf("The priority of thread %s is changed from L to M.\n", temp->name);
    }

    // reschedule if needed by schedule alarm

    running_thread = NULL;

    set_dispatch();
    
}


void OS2021_ThreadSetEvent(int event_id)
{
    // printf("Set Event\n");
    if (!waiting_queue[event_id][0]->front &&
    !waiting_queue[event_id][1]->front &&
    !waiting_queue[event_id][2]->front )
    {
        return; // there is no one in the ready queue
    }


    for(int i = 0; i < 3; i++)
    {
        if (waiting_queue[event_id][i]->front == NULL) continue;
    
        thread *temp = waiting_queue[event_id][i]->front;
        // thread * insert = (thread*)malloc(sizeof(thread));

        temp->state = "READY";
        temp->isWaitingEvent = 0;
        temp->waitingEvent = -1;
        temp->time_quantum = 0;

        printf("[DEQUEUE]%s\n", waiting_queue[event_id][i]->front->name);
        dequeue(waiting_queue[event_id][i]);
        printf("[ENQUEUE]%s\n", temp->name);

        enqueue(ready_queue[i], temp);

        printf("%s changes the status of %s to READY.\n", running_thread->name, temp->name);
        return;
    }
}

void OS2021_ThreadWaitTime(int msec)
{
    // printf("%s call the wait time.\n", running_thread->name);
    
    // printf("===READY===\n");
    // printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    // // print thread stuff
    // thread *temp = NULL;
    // for (int i = 0; i < 3; i++)
    // {
    //     temp = ready_queue[i]->front;
    //     while(temp)
    //     {
    //         printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
    //         temp = temp->next;
    //     }
    // }
    printf("=================\n");


    // Up the priority of the thread
    if (!strcmp(running_thread->priority, "H"))
    {
        printf(">%s\n", ready_queue[0]->front->name);
        dequeue(ready_queue[0]);
        enqueue(ready_queue[0], running_thread);
        printf(">%s\n", ready_queue[0]->front->name);

    }
    if (!strcmp(running_thread->priority, "M"))
    {
        printf("The priority of thread %s is changed from M to H.\n", running_thread->name);
        printf(">%s\n", ready_queue[1]->front->name);

        dequeue(ready_queue[1]);
        enqueue(ready_queue[0], running_thread);
        printf(">%s\n", ready_queue[0]->rear->name);

        printf("%s is at the front of the ready queue M\n", running_thread->name);

    }
    if (!strcmp(running_thread->priority, "L"))
    {
        printf("The priority of thread %s is changed from L to M.\n", running_thread->name);
        printf(">%s\n", ready_queue[2]->front->name);

        dequeue(ready_queue[2]);
        enqueue(ready_queue[1], running_thread);
        printf(">%s\n", ready_queue[0]->rear->name);

    }

    // Running thread changes its state to READY
    // Change the state fo the suspended task to READY after 10msec *10ms
    running_thread->state = "WAITING";
    running_thread->waitTime = (msec) * 10;
    running_thread->isWaiting = 1;

    running_thread->waitTimeCount = 0;


    // printf("===WAITING===\n");
    // printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    // // print thread stuff
    // for(int j=0; j<8; j++){
    //     for (int i = 0; i < 3; i++)
    //     {
    //         temp = waiting_queue[j][i]->front;
    //         while(temp)
    //         {
    //             printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
    //             temp = temp->next;
    //         }
    //     }
    // }
    // printf("=====\n");



    // printf("%s call the wait time.\n", running_thread->name);

    // Reschedule
    set_dispatch();

}

void OS2021_DeallocateThreadResource()
{
    /* Free the memory when the state of the thread is terminated. */
    struct thread *temp = NULL;
    struct thread *prev_temp = NULL;

    // printf("===READY===\n");
    // printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    // // print thread stuff
    // // thread *temp = NULL;
    // for (int i = 0; i < 3; i++)
    // {
    //     temp = ready_queue[i]->front;
    //     while(temp)
    //     {
    //         printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
    //         temp = temp->next;
    //     }
    // }
    // printf("=====\n");

    temp = NULL;

    // Search for the ready queue.
    for(int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if (!strcmp(temp->state,"TERMINATED"))
            {
                if(temp == ready_queue[i]->front)
                {  // free the first element
                    printf("%s\n","first element");
                    ready_queue[i]->front = ready_queue[i]->front->next;
                    printf("%s has been delete.\n", temp->name);
                    free(temp);
                    temp = ready_queue[i]->front;
                }
                else
                {
                    printf("ready to fail.\n");
                    // not the first element
                    printf("%s has been delete.\n", temp->name);
                    prev_temp->next = temp->next;
                    free(temp);

                    temp =  prev_temp;
                }
            }
            if(temp == NULL)
                    break;
            prev_temp = temp;
            temp = temp->next;
        }
    }

    // Search for the waiting queue.
    for(int j = 0; j < 8; j++)
    {
        for(int i = 0; i < 3; i++)
        {
            prev_temp = NULL; // init prev_temp in each queue
            if (waiting_queue[j][i]->front == NULL) continue;
            temp = waiting_queue[j][i]->front;
            while(temp)
            {
                if (!strcmp(temp->state,"TERMINATED"))
                {
                    if(temp == ready_queue[i]->front)
                    {  // free the first element
                        printf("%s\n","first element");
                        ready_queue[i]->front = ready_queue[i]->front->next;
                        printf("%s has been delete.\n", temp->name);
                        free(temp);
                        temp = ready_queue[i]->front;
                    }
                    else
                    {
                        printf("ready to fail.\n");
                        // not the first element
                        printf("%s has been delete.\n", temp->name);
                        prev_temp->next = temp->next;
                        free(temp);

                        temp =  prev_temp;
                    }
                }
                if(temp == NULL)
                    break;
                prev_temp = temp;
                temp = temp->next;
            }
        }
    }


    // temp = NULL;
    // printf("===WAITING===\n");
    // printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    // // print thread stuff
    // for(int j=0; j<8; j++){
    //     for (int i = 0; i < 3; i++)
    //     {
    //         temp = waiting_queue[j][i]->front;
    //         while(temp)
    //         {
    //             printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
    //             temp = temp->next;
    //         }
    //     }
    // }
    // printf("=====\n");
}

void OS2021_TestCancel()
{
    /* Free the memory when the state of the thread is terminated. */
    struct thread *temp = NULL;
    struct thread *prev_temp = NULL;
    int runningThreadIsCancel = 0;
    // Search for the ready queue.
    for(int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue; // make sure will not be NULL
        temp = ready_queue[i]->front;
        // prev_temp = ready_queue[i]->front;
        while(temp!=NULL)
        {
            if (temp->isCancel)
            {
                if(!strcmp(temp->state, "RUNNING"))
                {
                    // set_dispatch();
                    runningThreadIsCancel = 1;
                }
                printf("The memory space by %s has been released.\n", temp->name);
                
                if(temp == ready_queue[i]->front)
                {  // free the first element
                    printf("%s\n","first element");
                    ready_queue[i]->front = ready_queue[i]->front->next;
                    printf("%s has been delete.\n", temp->name);
                    free(temp);
                    temp = ready_queue[i]->front;
                }
                else
                {
                    printf("ready to fail.\n");
                    // not the first element
                    printf("%s has been delete.\n", temp->name);
                    prev_temp->next = temp->next;
                    free(temp);

                    temp =  prev_temp;
                }
            }
            if(temp == NULL)
                break;
            
            prev_temp = temp;
            temp = temp->next;
            
        }
    }
    

    // Search for the waiting queue.
    for(int j = 0; j < 8; j++)
    {
        for(int i = 0; i < 3; i++)
        {
            prev_temp = NULL; // init prev_temp in each queue
            if (waiting_queue[j][i]->front == NULL) continue;
            temp = waiting_queue[j][i]->front;
            // printf("In Wait Memory release\n");
            while(temp)
            {
                if (temp->isCancel)
                {

                    if(!strcmp(temp->state, "RUNNING"))
                    {
                        runningThreadIsCancel = 1;
                    }
                    // printf("The memory space by %s has been released\n", temp->name);
                    if(temp == waiting_queue[j][i]->front)
                    {  //free the firs element
                        waiting_queue[j][i]->front =  waiting_queue[j][i]->front->next;
                        free(temp);
                        temp = waiting_queue[j][i]->front;
                    }
                    else
                    { //not the first element
                       prev_temp->next = temp->next;
                       free(temp);
                       temp =  prev_temp;
                    }
                }
                if(temp == NULL)
                    break;
                prev_temp = temp;
                temp = temp->next;
            }
        }
    }

    if (runningThreadIsCancel)
    {
        set_dispatch();
    }
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    // printf("in createcontext\n");
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}


void Dispatcher()
{
    // printf("in the dispather\n");
    // If no one is running, places READY to RUNNING state
        struct thread *temp = NULL;
        bool found = false;

        for (int i = 0; i < 3; i++)
        {
            // if(ready_queue[i]->front == NULL) continue;
            temp = ready_queue[i]->front;
            while(temp)
            {
                if(!strcmp(temp->state, "READY"))
                {
                    running_thread = temp;
                    // printf("entry func is %s.\n", running_thread->entryFunction);
                    found = true;
                    break;
                }
                temp = temp->next;
            }
            if(found) break;
        }

        // printf("end of finding the ready state queue.\n");
        running_thread->state = "RUNNING";
        printf("the running thread change to %s\n", running_thread->name);
        // printf("%s\n",running_thread->state);
        func_string = running_thread->entryFunction;
        // printf("%s is running\n", running_thread->name);


        


        if(!strcmp(func_string, "Function1"))
            context_num = 0;
        else if(!strcmp(func_string, "Function2"))
            context_num = 1;
        else if(!strcmp(func_string, "Function3"))
            context_num = 2;
        else if(!strcmp(func_string, "Function4"))
            context_num = 3;
        else if(!strcmp(func_string, "Function5"))
            context_num = 4;
        else if(!strcmp(func_string, "ResourceReclaim"))
            context_num = 5;
    // printf("%d\n",context_num);
    setcontext(&contexts[context_num]);
}

void set_dispatch()
{
    if(context_num == 5)
    {
        setcontext(&dispatch_context);
    }
    else
    {
        swapcontext(&contexts[context_num], &dispatch_context);
    }
}

void StartSchedulingSimulation()
{
    /*Set Timer*/
    Signaltimer.it_interval.tv_usec = 10000;
    Signaltimer.it_interval.tv_sec = 0;
    ResetTimer();
    /*Create Context*/
    CreateContext(&dispatch_context, &timer_context, &Dispatcher);
    CreateContext(&contexts[0], &dispatch_context, &Function1);
    CreateContext(&contexts[1], &dispatch_context, &Function2);
    CreateContext(&contexts[2], &dispatch_context, &Function3);
    CreateContext(&contexts[3], &dispatch_context, &Function4);
    CreateContext(&contexts[4], &dispatch_context, &Function5);
    CreateContext(&contexts[5], &dispatch_context, &ResourceReclaim);
    setcontext(&dispatch_context);
}

// /* state */

// Check if any threads are in RUNNING state
int check_running()
{
    int flag = 0;

    struct thread *temp = NULL;

    for (int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if(!strcmp(temp->state, "RUNNING"))
            {
                flag = 1;
                break;
            }
            temp = temp->next;
        }
    }
    return flag;
}


/* queue */

void enqueue(struct queue *q, struct thread *t)
{
    if (q->front == NULL)
    {
        q->front = t;
        q->rear = t;
        return;
    }
    q->rear->next = t;
    q->rear = t;
    q->rear->next = NULL;
}


void dequeue(struct queue *q)
{
    // struct thread *front = q->front;

    if (q->front == NULL) return;
    struct thread *temp = q->front->next;
    q->front->next = NULL;
    q->front = temp;
    if(q->front == NULL) q->rear = NULL;
}


