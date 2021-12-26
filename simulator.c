#include "os2021_thread_api.h"
#include <json.h>

void add_time()
{
    struct thread *temp = NULL;
    // ready queue
    for (int i = 0; i < 3; i++)
    {
        if (ready_queue[i] == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if(!strcmp(temp->state, "RUNNING"))
            {
                temp->time_quantum += 10;

            }
            if(temp->isWaiting)
            {
                temp->waitTimeCount += 10;
                temp->w_time += 10;
            }


            temp = temp->next;
        }
    }
    // waiting queue
    for(int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            // if (waiting_queue[j][i] == NULL) continue;
            temp = waiting_queue[j][i]->front;
            while(temp)
            {
                temp->w_time += 10;

                temp = temp->next;
            }
        }
    }
}

// Return 1 if expire
int check_expire(struct thread *temp, char *priority)
{
    if(!strcmp(priority, "H"))
    {
        if(temp->time_quantum >= 100)
        {
            return 1;
        }
    }
    else if(!strcmp(priority, "M"))
    {
        if(temp->time_quantum >= 200)
        {
            return 1;
        }
    }
    else if(!strcmp(priority, "L"))
    {
        if(temp->time_quantum >= 300)
        {
            return 1;
        }
    }
    return 0;
}


void check_time()
{
    struct thread *temp = NULL;

    for (int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;

        while(temp)
        {
            // printf("%s\n",temp->state);
            if(!strcmp(temp->state, "RUNNING"))
            {

                if(check_expire(temp, temp->priority))
                {
                    temp->state = "READY";
                    temp->time_quantum = 0;


                    switch(i)
                    {
                    case 0:
                    {
                        temp->priority = "M";
                        // printf("###%s\n", temp->name);
                        dequeue(ready_queue[0]);
                        enqueue(ready_queue[1], temp);


                    }
                    case 1:
                    {
                        temp->priority = "L";

                        dequeue(ready_queue[1]);
                        enqueue(ready_queue[2], temp);


                    }
                    case 2:
                    {
                        temp->priority = "L";

                        dequeue(ready_queue[2]);
                        enqueue(ready_queue[2], temp);
                        // printf("###%s\n", ready_queue[2]->rear->name);
                        // printf("###%s\n", temp->name);

                    }
                    }

                    set_dispatch();
                    return;
                }
            }
            temp = temp->next;
        }

    }
}

// int wake_scheduler()
// {
//     // check if scheduler needs to be woken
//     int flag = 0;

//     struct thread *temp = NULL;

//     for (int i = 0; i < 3; i++)
//     {
//         if (ready_queue[i] == NULL) continue;
//         temp = ready_queue[i]->front;
//         while(temp)
//         {
//             if(temp->isScheduled)
//             {
//                 scheduled_thread = temp;
//                 flag = 1;
//             }
//             temp = temp->next;
//         }
//     }

//     return flag;
// }

// void scheduler()
// {
//     printf("SCHEDULING!\n");
//     scheduled_thread->isScheduled = 0;
//     scheduled_thread->state = "READY";
//     scheduled_thread = NULL;
// }

void check_waiting_time()
{
    struct thread *temp = NULL;

    for (int i = 0; i < 3; i++)
    {
        if(ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if(temp->isWaiting == 1)
            {
                // printf("in check waiting time. %d %d\n", temp->waitTimeCount, temp->waitTime);

                if(temp->waitTimeCount >= temp->waitTime)
                {
                    temp->state = "READY";
                    temp->isWaiting = 0;
                    temp->waitTime = 0;
                    temp->waitTimeCount = 0;
                }
            }
            temp = temp->next;
        }
    }
}

int checkThreadWaiting()
{
    int flag = 0;

    struct thread *temp = NULL;

    for (int i = 0; i < 3; i++)
    {
        if (ready_queue[i]->front == NULL) continue;
        temp = ready_queue[i]->front;
        while(temp)
        {
            if(temp->isWaiting == 1)
            {
                flag = 1;
                break;
            }
            temp = temp->next;
        }

    }
    // printf("in thread waiting.\n");
    return flag;
}

void sigalrm_handler(int signum)
{
    signal(SIGALRM, sigalrm_handler);
    // printf("Timer\n");
    // Add time to all the thread
    add_time();

    // Check if any threads need to switch states
    // If TQ runs out, switch state from RUNNING to READY
    check_time();

    // If it is waiting, switch state from WAITING to READY
    if(checkThreadWaiting())
        check_waiting_time();

    // // Decide whether to wake scheduler
    // if(wake_scheduler())
    //     scheduler();
}

void sigtstp_handler(int signum)
{
    // reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sigtstp_handler);

    printf("\n");
    printf("*******************************************************************************\n");
    printf("*\tTID\tName\t\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time\t*\n");
    // print thread stuff
    thread *temp = NULL;
    for (int i = 0; i < 3; i++)
    {
        temp = ready_queue[i]->front;
        while(temp)
        {
            printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
            temp = temp->next;
        }
    }
    // printf("*******************************************************************************\n");

    for(int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            temp = waiting_queue[j][i]->front;
            while(temp)
            {
                printf("*\t%d\t%s\t\t%s\t%s\t\t%s\t\t%d\t%d\t*\n", temp->tid, temp->name, temp->state, temp->base_priority, temp->priority, temp->time_quantum, temp->w_time);
                temp = temp->next;
            }
        }
    }
    printf("*******************************************************************************\n");
}

void init()
{
    /* init queue */
    for(int i = 0; i < 3; i++)
    {
        ready_queue[i] = (queue*)malloc(sizeof(queue));
        ready_queue[i]->front = NULL;
        ready_queue[i]->rear = NULL;
    }

    // Search for the waiting queue.
    for(int j = 0; j < 8; j++)
    {
        for(int i = 0; i < 3; i++)
        {
            waiting_queue[j][i] = (queue*)malloc(sizeof(queue));
            waiting_queue[j][i]->front = NULL;
            waiting_queue[j][i]->rear = NULL;
        }

    }
    /* init thread */
    // running_thread = (thread*)malloc(sizeof(thread));
    // running_thread->name = "";
    // running_thread->entryFunction = "";
    // running_thread->priority = "";
    // running_thread->base_priority ="";
    // running_thread->cancelMode = 0;
    // running_thread->tid = 0;
    // running_thread->state = "";
    // running_thread->isCancel = 0;
    // running_thread->q_time = 0;
    // running_thread->w_time = 0;
    // running_thread->time_quantum = 0;
    // running_thread->isScheduled = 0;
    // running_thread->isCancel = 0;
    // running_thread->waitingEvent= 0;
    // running_thread->isWaiting = 0;
    // running_thread->waitTime = 0;
    // running_thread->waitTimeCount = 0;
}

int main(int argc,char** argv)
{
    /* Parse the json file */
    FILE *fp;
    char buffer[409600]; // Store the content of the json file.

    struct json_object *parsed_json = NULL;
    struct json_object *threads = NULL;
    struct json_object *thread = NULL;
    struct json_object *name = NULL;
    struct json_object *entryFunction = NULL;
    struct json_object *priority = NULL;
    struct json_object *cancelMode = NULL;


    fp = fopen("init_threads.json", "r");
    fread(buffer, 409600, 1, fp);
    fclose(fp);

    parsed_json = json_tokener_parse(buffer);
    json_object_object_get_ex(parsed_json, "Threads", &threads);
    // printf("%s", json_object_get_string(threads));
    int numOfThreads = json_object_array_length(threads);

    init();

    for(int i = 0; i < numOfThreads; i++)
    {
        thread = json_object_array_get_idx(threads,i);
        json_object_object_get_ex(thread, "name", &name);
        json_object_object_get_ex(thread, "entry function", &entryFunction);
        json_object_object_get_ex(thread, "priority", &priority);
        json_object_object_get_ex(thread, "cancel mode", &cancelMode);

        // printf("%s, %s, %s, %d\n", json_object_get_string(name), json_object_get_string(entryFunction), json_object_get_string(priority), atoi(json_object_get_string(cancelMode)));

        /* Create the threads */
        OS2021_ThreadCreate(json_object_get_string(name), json_object_get_string(entryFunction), json_object_get_string(priority), atoi(json_object_get_string(cancelMode)));
    }

    // Create reclaimer.
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);

    // handles signals
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGALRM, sigalrm_handler);

    // printf("ready to get inside the startschedulingsimulation\n");
    StartSchedulingSimulation();


    return 0;
}

