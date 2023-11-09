#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
int special_flag[100];
struct customernode
{
    int cust_no;
    int total_orders;
    int cust_arrival;
    int order_completion_status;
};
struct ordernode
{
    int order_taken_status;
    int orderstatus;
    int cust_no;
    int totaltoppings;
    char *flavour;
    char **toppings;
};
int tmr = 0;
int totalorders = 0;
// int totalcustomers = 0;
int current_customers = 0;
struct ordernode order_list[500];
struct customernode cust[100];
sem_t capacity;
sem_t order_comp[100];
sem_t machines[100];
sem_t customers[100];
sem_t rej_customers[100];
sem_t machine_lock;
int finish_time_order[100];
// sem_t order_number;
// sem_t machine_number;
sem_t superlock;
// sem_t order_completion_time[500];
int rejection_cust[100];
struct arglist
{
    char **flavour_list;
    int *flavour_time;
    char **topping_list;
    int *topping_stock;
    int *machine_start;
    int *machine_end;
    int f;
    int t;
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
        order_list[i].flavour = (char *)malloc(sizeof(char) * 20);
        order_list[i].cust_no = -1;
        order_list[i].totaltoppings = 0;
        order_list[i].toppings = (char **)malloc(sizeof(char *) * 50);
        for (int j = 0; j < 50; j++)
            order_list[i].toppings[j] = (char *)malloc(sizeof(char) * 20);
    }
}
int close_sig = 0;

int order_status_checker(int cust_no)
{
    for (int i = 0; i < totalorders; i++)
    {
        if (order_list[i].cust_no == cust_no && order_list[i].orderstatus == 0)
            return 0;
    }
    return 1;
}
bool remaining(int *machines_remaining, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (machines_remaining[i] == 0)
        {
            return 1;
        }
    }
    return 0;
}
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
void *machine_simulation(void *arg)
{
    struct arglist *aglst = (struct arglist *)arg;
    sem_wait(&machines[aglst->work_index]);
    printf("Machine %d started at %d sec\n", aglst->work_index + 1, tmr);
    int cst = 0;
    int flag = -1;
    while (1)
    {
        sem_wait(&machines[aglst->work_index]);
        if (close_sig == 1)
            return NULL;
        if (flag == -1)
        {
            sem_wait(&machine_lock);
            // sem_wait(&order_number);
            for (; cst < totalorders; cst++)
            {
                // int reject_flag = 0;
                if (cust[order_list[cst].cust_no].order_completion_status == 0 && order_list[cst].order_taken_status == 0)
                {
                    
                    int findex = give_index(aglst->flavour_list, aglst->f, order_list[cst].flavour);
                    int ftime = aglst->flavour_time[findex];
                    // printf("machine %d checking order %d\n", aglst->work_index + 1, cst);
                    int totaltop = order_list[cst].totaltoppings;
                    for (int j = 0; j < totaltop; j++)
                    {
                        int tindex = give_index(aglst->topping_list, aglst->t, order_list[cst].toppings[j]);
                        if (aglst->topping_stock[tindex] == 0)
                        {
                            // reject_flag = 1;
                            order_list->order_taken_status = 1;
                            order_list->orderstatus = 1;
                            rejection_cust[order_list[cst].cust_no] = 1;
                            sem_post(&rej_customers[order_list[cst].cust_no]);
                            continue;
                        }
                    }

                    if ((cust[order_list[flag].cust_no].cust_arrival == tmr && aglst->machine_end[aglst->work_index] - tmr >= ftime + 1) || (cust[order_list[flag].cust_no].cust_arrival < tmr && aglst->machine_end[aglst->work_index] - tmr >= ftime))
                    {
                        order_list[cst].order_taken_status = 1;

                        for (int j = 0; j < totaltop; j++)
                        {
                            int tindex = give_index(aglst->topping_list, aglst->t, order_list[cst].toppings[j]);
                            aglst->topping_stock[tindex] -= 1;
                        }
                        if (cust[order_list[cst].cust_no].cust_arrival == tmr)
                        {
                            finish_time_order[aglst->work_index] = tmr + ftime + 1;
                            special_flag[aglst->work_index] = 1;
                        }
                        else
                            finish_time_order[aglst->work_index] = tmr + ftime;
                        flag = cst;
                        cst++;
                        break;
                    }
                }
            }
            sem_post(&machine_lock);
            if (flag != -1)
            {
                // printf("%d %d %d\n",flag,cust[order_list[flag].cust_no].cust_arrival,tmr);

                if (cust[order_list[flag].cust_no].cust_arrival == tmr)
                    continue;
            }
        }
        if (flag != -1)
        {

            printf("Machine %d started preparing order no. %d at time %d\n", aglst->work_index + 1, flag + 1, tmr);
            sem_wait(&order_comp[aglst->work_index]);
            order_list[flag].orderstatus = 1;
            printf("Order no. %d completed by machine %d at time %d\n", flag + 1, aglst->work_index + 1, tmr);
            if (order_status_checker(order_list[flag].cust_no))
            {
                sem_post(&rej_customers[order_list[flag].cust_no]);
                sem_post(&customers[order_list[flag].cust_no]);
            }
            flag = -1;
        }
        // else
        // {
        //     sem_post(&order_number);
        // }
    }
    return NULL;
}
void *customer_waiting(void *arg)
{

    struct arglist *aglst = (struct arglist *)arg;
    // sem_wait(&capacity);
    sem_wait(&customers[aglst->work_index]);
    sem_wait(&superlock);
    printf("Customer %d enters at time %d sec\n", aglst->work_index, tmr);
    current_customers++;
    // sem_wait(&superlock);
    // sem_post(&order_number);
    totalorders =totalorders+cust[aglst->work_index].total_orders;
    sem_post(&superlock);
    sem_post(&machine_lock);
    sem_wait(&rej_customers[aglst->work_index]);
    if (rejection_cust[aglst->work_index] == 1)
    {
        cust[aglst->work_index].order_completion_status = 1;
        printf("Order of customer %d got rejected due to ingredient shortage\n", aglst->work_index);
        // sem_post(&capacity);
        current_customers--;
        return NULL;
    }
    sem_wait(&customers[aglst->work_index]);
    cust[aglst->work_index].order_completion_status = 1;
    printf("Customer %d leaves with it's complete order at time %d sec\n", aglst->work_index, tmr);
    // sem_post(&capacity);
    current_customers--;
    return NULL;
}
// int customer_incoming(int* cust_arrival)
// {

// }
int main()
{
    init_ordernode();
    int n, k, f, t;
    scanf("%d %d %d %d", &n, &k, &f, &t);
    int machines_remaining[n];
    int machine_start[n];
    int machine_end[n];
    int max = 0;
    for (int i = 0; i < n; i++)
    {
        scanf("%d %d", &machine_start[i], &machine_end[i]);
        if (machine_end[i] > max)
            max = machine_end[i];
    }
    char **flavour_list = (char **)malloc(sizeof(char *) * f);
    for (int i = 0; i < f; i++)
        flavour_list[i] = (char *)malloc(sizeof(char) * 20);
    int flavour_time[f];
    char **topping_list = (char **)malloc(sizeof(char *) * t);
    for (int i = 0; i < t; i++)
        topping_list[i] = (char *)malloc(sizeof(char) * 20);
    int topping_stock[t];
    for (int i = 0; i < f; i++)
        scanf("%s %d", flavour_list[i], &flavour_time[i]);
    // for(int i=0;i<f;i++)
    // printf("%s\n",flavour_list[i]);
    for (int i = 0; i < t; i++)
        scanf("%s %d", topping_list[i], &topping_stock[i]);
    int num_cust;
    scanf("%d", &num_cust);
    int count = 0;
    for (int i = 1; i <= num_cust; i++)
    {
        int num_t;
        scanf("%d %d %d", &cust[i].cust_no, &cust[i].cust_arrival, &cust[i].total_orders);
        cust[i].order_completion_status = 0;
        for (int j = 0; j < cust[i].total_orders; j++)
        {
            scanf("%d", &num_t);
            order_list[count].cust_no = cust[i].cust_no;
            scanf("%s", order_list[count].flavour);
            for (int k = 0; k < num_t; k++)
                scanf("%s", order_list[count].toppings[k]);
            count++;
        }
    }
    sem_init(&capacity, 0, k);
    sem_init(&superlock, 0, 1);
    sem_init(&machine_lock, 0, 1);
    for (int i = 0; i < 100; i++)
    {
        sem_init(&customers[i], 0, 0);
        sem_init(&machines[i], 0, 0);
        sem_init(&order_comp[i], 0, 0);
        sem_init(&rej_customers[i], 0, 0);
        finish_time_order[i] = -1;
        rejection_cust[i] = 0;
        special_flag[i] = 0;
    }
    struct arglist arg[n];
    pthread_t machine_threads[n];
    pthread_t customer_threads[num_cust];
    for (int i = 0; i < n; i++)
    {
        arg[i].f = f;
        arg[i].flavour_list = flavour_list;
        arg[i].flavour_time = flavour_time;
        arg[i].k = k;
        arg[i].machine_end = machine_end;
        arg[i].machine_start = machine_start;
        arg[i].n = n;
        arg[i].t = t;
        arg[i].topping_list = topping_list;
        arg[i].topping_stock = topping_stock;
        arg[i].work_index = i;
        pthread_create(&machine_threads[i], NULL, machine_simulation, &arg[i]);
    }
    struct arglist arg2[num_cust + 1];
    for (int i = 1; i <= num_cust; i++)
    {
        arg2[i].f = f;
        arg2[i].flavour_list = flavour_list;
        arg2[i].flavour_time = flavour_time;
        arg2[i].k = k;
        arg2[i].machine_end = machine_end;
        arg2[i].machine_start = machine_start;
        arg2[i].n = n;
        arg2[i].t = t;
        arg2[i].topping_list = topping_list;
        arg2[i].topping_stock = topping_stock;
        arg2[i].work_index = i;
        pthread_create(&customer_threads[i], NULL, customer_waiting, &arg2[i]);
    }
    int returned_count = 0;
    while (tmr <= max)
    {
        sem_wait(&machine_lock);
        int new_enter_flag = 0;
        for (int i = 1; i <= num_cust; i++)
        {
            sem_wait(&superlock);
            if (cust[i].cust_arrival == tmr)
            {
                if (current_customers == k)
                {
                    returned_count++;
                    sem_post(&superlock);
                    break;
                }
                else
                {
                    new_enter_flag = 1;
                    sem_post(&customers[i]);
                    sem_post(&superlock);
                }
            }
            else
            {
                sem_post(&superlock);
            }
        }
        if (new_enter_flag == 0)
            sem_post(&machine_lock);
        else
            new_enter_flag = 0;
        for (int i = 0; i < n; i++)
        {
            if (machine_start[i] == tmr)
            {
                sem_post(&machines[i]);
            }
        }
        for (int i = 0; i < n; i++)
        {
            if (machine_start[i] <= tmr && tmr <= machine_end[i] && (finish_time_order[i] <= tmr || special_flag[i] == 1))
            {
                sem_post(&machines[i]);
                special_flag[i] = 0;
            }
        }
        for (int i = 0; i < n; i++)
        {
            if (finish_time_order[i] == tmr)
                sem_post(&order_comp[i]);
        }
        sleep(1);
        for (int i = 0; i < n; i++)
        {
            if (machine_end[i] == tmr)
            {
                printf("Machine no %d stopped at %d sec\n", i + 1);
            }
        }
        tmr += 1;
    }
    printf("Shop closed\n");
    close_sig = 1;
    for (int i = 0; i < n; i++)
        sem_post(&machines[i]);
}

/*2 3 2 3
0 7
4 10
vanilla 3
chocolate 4
caramel 1
brownie 4
strawberry 4
2
1 1 2
1
vanilla caramel
2
chocolate brownie strawberry
2 2 1
2
vanilla strawberry caramel*/