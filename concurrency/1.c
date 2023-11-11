#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#define CYN "\e[0;36m"
#define RESET "\x1b[0m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define WHITE "\x1b[37m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define ORANGE "\e[38;2;255;85;0m"
#define BMAG "\e[1;35m"
int special_flag[100];
struct customernode
{
    int cust_no;
    int tol_time;
    int cust_arrival;
    int order_completion_status;
};
struct ordernode
{
    int order_taken_status;
    int orderstatus;
    int cust_no;
    char *cofee;
};
int tmr = 0;
int totalorders = 0;
// int totalcustomers = 0;
// int current_customers = 0;
// int returned_count = 0;
int coffee_wasted=0;
int wait_time=0;
int last_cust_to_enter_flag = 0;
struct ordernode order_list[500];
struct customernode cust[100];
sem_t capacity;
sem_t order_comp[100];
sem_t baristas[100];
sem_t customers[100];
sem_t rej_customers[100];
sem_t machine_lock;
int finish_time_order[100];
int wake_up_index[100];
int wake_up_index_cust[100];
// sem_t order_number;
// sem_t machine_number;
sem_t superlock;
// sem_t order_completion_time[500];
int tol_time[100];
struct arglist
{
    char **cofee_list;
    int *cofee_time;
    int b;
    int k;
    int n;
    int work_index;
};
void init_ordernode()
{
    for (int i = 0; i < 500; i++)
    {
        order_list[i].order_taken_status = 0;
        order_list[i].orderstatus = 0;
        order_list[i].cofee = (char *)malloc(sizeof(char) * 20);
        order_list[i].cust_no = -1;
    }
}
int close_sig = 0;
int give_index2(char **array, int size, char *string)
{
    for (int i = 1; i <= size; i++)
    {
        if (strcmp(array[i], string) == 0)
            return i;
    }
    return -1;
}
int give_index(char **array, int size, char *string)
{
    for (int i = 0; i < size; i++)
    {
        if (strcmp(array[i], string) == 0)
            return i;
    }
    return -1;
}
int free_barista(int no,int b)
{
    for(int i=0;i<b;i++)
    {
        if(finish_time_order[i]<=tmr || special_flag[i]==1)
        {
            no--;
            special_flag[i]=0;
            if(no==0){
            return i;
            }
        }
    }
    return -1;
}

int to_enter_customer(int no, int cust_cnt)
{
    for (int i = 1; i <= cust_cnt; i++)
    {
        if (cust[i].cust_arrival == tmr)
        {
            no--;
            if (no == 0)
                return i;
        }
    }
    return -1;
}

void *barista_simulation(void *arg)
{
    struct arglist *aglst = (struct arglist *)arg;
    int cst = 0;
    int flag = -1;
    while (1)
    {
        sem_wait(&baristas[aglst->work_index]);
        if (close_sig == 1)
            return NULL;
        if (flag == -1)
        {
            sem_wait(&machine_lock);
            if (wake_up_index[aglst->work_index] != -1)
            {
                int temp=wake_up_index[aglst->work_index];
                wake_up_index[aglst->work_index]=-1;
                sem_post(&baristas[temp]);
            }
            // printf("hello by machine %d\n",aglst->work_index);
            // sem_wait(&order_number);
            for (; cst < totalorders; cst++)
            {

                // int reject_flag = 0;

                // printf("order no %d of cust %d is serverd %d\n",order_list[cst].order_no_cust,order_list[cst].cust_no,cust[order_list[cst].cust_no].order_completion_status);
                if (cust[order_list[cst].cust_no].order_completion_status == 0 && order_list[cst].order_taken_status == 0)
                {
                    // printf("hello by machine %d checking order_no %d of cust %d\n",aglst->work_index,order_list[cst].order_no_cust,order_list[cst].cust_no);
                    int findex = give_index(aglst->cofee_list, aglst->k, order_list[cst].cofee);
                    int ftime = aglst->cofee_time[findex];
                    order_list[cst].order_taken_status = 1;
                        // printf("hello barista %d at time %d orderno %d\n",aglst->work_index+1,tmr,cst);
                    if (cust[order_list[cst].cust_no].cust_arrival == tmr)
                    {
                        finish_time_order[aglst->work_index] = tmr + ftime + 1; 
                    }
                    else
                        finish_time_order[aglst->work_index] = tmr + ftime+1;
                    special_flag[aglst->work_index]=1;
                    flag = cst;
                    cst++;
                    break;
                }
            }
            sem_post(&machine_lock);
        }
        else if (flag != -1)
        {
            // if(wake_up_index[aglst->work_index]!=-1)
            // sem_post(&baristas[wake_up_index[aglst->work_index]]);
             if (wake_up_index[aglst->work_index] != -1)
            {
                int temp=wake_up_index[aglst->work_index];
                wake_up_index[aglst->work_index]=-1;
                sem_post(&baristas[temp]);
            }
            if(cust[order_list[flag].cust_no].order_completion_status==1)
            {
                flag=-1;
                finish_time_order[aglst->work_index]=-1;
                continue;
            }
            printf(CYN "Barista %d starts preparing cofee of customer %d at %d seconds(s)\n" RESET, aglst->work_index + 1, order_list[flag].cust_no, tmr);
            sem_wait(&order_comp[aglst->work_index]);
            order_list[flag].orderstatus = 1;
            if(cust[order_list[flag].cust_no].order_completion_status==1)
            coffee_wasted++;
            cust[order_list[flag].cust_no].order_completion_status=1;
            printf(BLUE "Barista %d completes preparing coffee of customer %d at %d seconds(s)\n" RESET, aglst->work_index + 1,  order_list[flag].cust_no, tmr);
            sem_post(&customers[order_list[flag].cust_no]);
            flag = -1;
        }
    }
    return NULL;
}
int customer_remaining(int n)
{
    for(int i=1;i<=n;i++){
    if(cust[i].order_completion_status==0)
    return 1;
    }
    return 0;
}
void *customer_waiting(void *arg)
{

    struct arglist *aglst = (struct arglist *)arg;
    sem_wait(&customers[aglst->work_index]);
    
    totalorders = totalorders + 1;
    printf(YELLOW "Customer %d enters at time %d sec\n" RESET, aglst->work_index, tmr);
    printf(YELLOW "Customer %d orders a %s\n" RESET, aglst->work_index, order_list[aglst->work_index-1].cofee);
    
   if (wake_up_index_cust[aglst->work_index] != -1)
    {
        int temp = wake_up_index_cust[aglst->work_index];
        wake_up_index_cust[aglst->work_index] = -1;
        sem_post(&customers[temp]);
    }
    else
    {
        sem_post(&machine_lock);
    }
    tol_time[aglst->work_index]=cust[aglst->work_index].cust_arrival+cust[aglst->work_index].tol_time+1;
    sem_wait(&customers[aglst->work_index]);
    if(cust[aglst->work_index].order_completion_status==0)
    {
        wait_time+=cust[aglst->work_index].tol_time+1;
        printf(RED"Customer %d leaves without their order at %d second(s)\n"RESET,aglst->work_index,tmr);
        cust[aglst->work_index].order_completion_status=1;
        return NULL;
    }
    wait_time+=tmr-cust[aglst->work_index].cust_arrival-aglst->cofee_time[give_index(aglst->cofee_list,aglst->k,order_list[aglst->work_index-1].cofee)];
    printf(GREEN "Customer %d has collected their order(s) and left at %d second(s)\n" RESET, aglst->work_index, tmr);
    return NULL;
}
int main()
{
    char *input1 = malloc(sizeof(char) * 512);
    init_ordernode();
    int b, k, n;
    fgets(input1, 512, stdin);
    sscanf(input1, "%d %d %d", &b, &k, &n);
    int max = 0;
    char **cofee_list = (char **)malloc(sizeof(char *) * k);
    for (int i = 0; i < k; i++)
        cofee_list[i] = (char *)malloc(sizeof(char) * 20);
    int cofee_time[k];
    for (int i = 0; i < k; i++)
    {
        fgets(input1, 512, stdin);
        sscanf(input1, "%s %d", cofee_list[i], &cofee_time[i]);
    }
    char *input = (char *)malloc(sizeof(char) * 10);
    for (int i = 0; i < n; i++)
    {
        fgets(input1, 512, stdin);
        sscanf(input1, "%d %s %d %d", &order_list[i].cust_no, order_list[i].cofee, &cust[i + 1].cust_arrival, &cust[i + 1].tol_time);
        cust[i].cust_no = order_list[i].cust_no;
        cust[i].order_completion_status = 0;
    }

    sem_init(&superlock, 0, 1);
    sem_init(&machine_lock, 0, 1);
    for (int i = 0; i < 100; i++)
    {
        sem_init(&customers[i], 0, 0);
        sem_init(&baristas[i], 0, 0);
        sem_init(&order_comp[i], 0, 0);
        sem_init(&rej_customers[i], 0, 0);
        finish_time_order[i] = -1;
        tol_time[i] = -1;
        special_flag[i] = 0;
        wake_up_index[i]=-1;
        wake_up_index_cust[i]=-1;
    }
    struct arglist arg[b];
    pthread_t machine_threads[b];
    pthread_t customer_threads[n];
    for (int i = 0; i < b; i++)
    {
        arg[i].b = b;
        arg[i].cofee_list = cofee_list;
        arg[i].cofee_time = cofee_time;
        arg[i].k = k;
        arg[i].n = n;
        arg[i].work_index = i;
        pthread_create(&machine_threads[i], NULL, barista_simulation, &arg[i]);
    }
    struct arglist arg2[n + 1];
    for (int i = 1; i <= n; i++)
    {
        arg2[i].b = b;
        arg2[i].cofee_list = cofee_list;
        arg2[i].cofee_time = cofee_time;
        arg2[i].k = k;
        arg2[i].n = n;
        arg2[i].work_index = i;
        pthread_create(&customer_threads[i], NULL, customer_waiting, &arg2[i]);
    }
    // int returned_count = 0;
    while (customer_remaining(n))
    {
         printf(BMAG "time: %d sec\n\n" RESET, tmr);
        sem_wait(&machine_lock);


       int count_cust = 1;
        int first_free_cust = to_enter_customer(count_cust++, n);
        int temp_cust = first_free_cust;
        while (temp_cust != -1)
        {
            int temp2_cust = to_enter_customer(count_cust++, n);
            wake_up_index_cust[temp_cust] = temp2_cust;
            temp_cust = temp2_cust;
        }
        if (first_free_cust != -1)
            sem_post(&customers[first_free_cust]);
        else
        {
            sem_post(&machine_lock);
        }


        int count=1;
        int first_free=free_barista(count++,b);
        int temp=first_free;
        while(temp!=-1)
        {
            int temp2=free_barista(count++,b);
            wake_up_index[temp]=temp2;
            temp=temp2;
        }
        if(first_free!=-1)
        sem_post(&baristas[first_free]);
        // for (int i = 0; i < b; i++)
        // {
        //     if(finish_time_order[i]<=tmr || special_flag[i]==1){
        //     sem_post(&baristas[i]);
        //     special_flag[i]=0;
        //     }
        // }
        for (int i = 0; i < b; i++)
        {
            if (finish_time_order[i] == tmr)
                sem_post(&order_comp[i]);
        }
       for(int i=1;i<=n;i++)
        {
            if(tol_time[i]==tmr)
            {
                sem_post(&customers[i]);
            }
        }
        sleep(1);
        tmr += 1;
        printf("\n");
    }
    printf(BMAG "time: %d sec\n\n" RESET, tmr);
    close_sig = 1;
    for (int i = 0; i < n; i++)
        sem_post(&baristas[i]);
    printf(RED"Coffee wasted: %d\n"RESET,coffee_wasted);
    printf(RED"Avg Wait Time: %d\n"RESET,wait_time/n);
    printf(WHITE "Cafe closed\n" RESET);
}
