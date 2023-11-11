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
int some_ing_exh = 0;
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
    int order_no_cust;
    int totaltoppings;
    char *flavour;
    char **toppings;
};
int tmr = 0;
int totalorders = 0;
// int totalcustomers = 0;
int current_customers = 0;
int returned_count = 0;
int last_cust_to_enter_flag = 0;
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
        order_list[i].order_no_cust = -1;
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
int ingreadient_checker(int *top_stock, int cst, char **topping_list, int total_top)
{
    if (cust[cst].order_completion_status == 1)
        return -1;
    for (int i = 0; i < totalorders; i++)
    {
        if (order_list[i].cust_no == cst)
        {
            for (int j = 0; j < order_list[i].totaltoppings; j++)
            {
                int tindex = give_index(topping_list, total_top, order_list[cst].toppings[j]);
                if (top_stock[tindex] == 0)
                    return 0;
                else if (top_stock[tindex] > 0 || top_stock[tindex] == -1)
                    continue;
                else if (top_stock[tindex] < -1)
                    return 0;
            }
        }
    }
    return 1;
}
int any_order_being_prepared(int order_cnt)
{
    for (int i = 0; i < order_cnt; i++)
    {
        if (order_list[i].order_taken_status == 1 && order_list[i].orderstatus == 0)
            return 0;
    }
    return 1;
}
int ingredient_exhausted_checker(int *topping_stock, int total)
{
    for (int i = 0; i < total; i++)
    {
        if (topping_stock[i]!=-1 && !(topping_stock[i] <= 0))
            return 0;
    }
    return 1;
}

void *machine_simulation(void *arg)
{
    struct arglist *aglst = (struct arglist *)arg;
    sem_wait(&machines[aglst->work_index]);
    printf(ORANGE "Machine %d started at %d sec\n" RESET, aglst->work_index + 1, tmr);
    int cst = 0;
    int flag = -1;
    while (1)
    {

        sem_wait(&machines[aglst->work_index]);
        if (close_sig == 1)
        {
            if(some_ing_exh==1)
            printf(RED"Machine %d turned off because some ingredient got exhausted\n"RESET,aglst->work_index+1);
            return NULL;
        }
        if (flag == -1)
        {
            sem_wait(&machine_lock);
            // printf("hello by machine %d\n",aglst->work_index);
            // sem_wait(&order_number);
            for (; cst < totalorders; cst++)
            {

                // int reject_flag = 0;

                // printf("order no %d of cust %d is serverd %d\n",order_list[cst].order_no_cust,order_list[cst].cust_no,cust[order_list[cst].cust_no].order_completion_status);
                if (cust[order_list[cst].cust_no].order_completion_status == 0 && order_list[cst].order_taken_status == 0)
                {
                    // printf("hello by machine %d checking order_no %d of cust %d\n",aglst->work_index,order_list[cst].order_no_cust,order_list[cst].cust_no);
                    int findex = give_index(aglst->flavour_list, aglst->f, order_list[cst].flavour);
                    int ftime = aglst->flavour_time[findex];
                    // printf("ftime:%d\n",ftime);
                    // printf("machine %d checking order %d\n", aglst->work_index + 1, cst);
                    int totaltop = order_list[cst].totaltoppings;
                    int topping_shortage_flag = 0;
                    for (int j = 0; j < totaltop; j++)
                    {
                        int tindex = give_index(aglst->topping_list, aglst->t, order_list[cst].toppings[j]);
                        // printf("current topping stock:%d\n", aglst->topping_stock[tindex]);
                        if (aglst->topping_stock[tindex] == 0 || aglst->topping_stock[tindex] < -1)
                        {
                            // reject_flag = 1;
                            order_list->order_taken_status = 1;
                            order_list->orderstatus = 1;
                            rejection_cust[order_list[cst].cust_no] = 1;
                            sem_post(&rej_customers[order_list[cst].cust_no]);
                            topping_shortage_flag = 1;
                            break;
                        }
                    }
                    if (topping_shortage_flag == 1)
                    {
                        topping_shortage_flag = 0;
                        continue;
                    }
                    // printf("arrival time : %d , tmr: %d && ftime:%d end_time_of_machine\n",cus[order_list[]])
                    if ((cust[order_list[cst].cust_no].cust_arrival == tmr && aglst->machine_end[aglst->work_index] - tmr >= ftime + 1) || (cust[order_list[cst].cust_no].cust_arrival < tmr && aglst->machine_end[aglst->work_index] - tmr >= ftime))
                    {
                        // printf("hello by machine %d checking order_no %d of cust %d\n",aglst->work_index,order_list[cst].order_no_cust,order_list[cst].cust_no);

                        order_list[cst].order_taken_status = 1;
                        // printf("totaltop=%d\n", totaltop);
                        for (int j = 0; j < totaltop; j++)
                        {
                            int tindex = give_index(aglst->topping_list, aglst->t, order_list[cst].toppings[j]);
                            if (aglst->topping_stock[tindex] == -1)
                                continue;
                            else if (aglst->topping_stock[tindex] > 0)
                                aglst->topping_stock[tindex] -= 1;
                            // printf("new toping for %s is %d\n",order_list[cst].toppings[j],aglst->topping_stock[tindex]);
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

            printf(CYN "Machine %d starts preparing ice cream %d of customer %d at %d seconds(s)\n" RESET, aglst->work_index + 1, order_list[flag].order_no_cust, order_list[flag].cust_no, tmr);
            sem_wait(&order_comp[aglst->work_index]);
            order_list[flag].orderstatus = 1;
            printf(BLUE "Machine %d completes preparing ice cream %d of customer %d at %d seconds(s)\n" RESET, aglst->work_index + 1, order_list[flag].order_no_cust, order_list[flag].cust_no, tmr);
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
    totalorders = totalorders + cust[aglst->work_index].total_orders;
    if (current_customers == aglst->k)
    {
        printf(RED"Customer %d left because parlor was full at time %d\n"RESET, cust[aglst->work_index], tmr);
        cust[aglst->work_index].order_completion_status = 1;
        returned_count++;
        sem_post(&superlock);
        if (last_cust_to_enter_flag == cust[aglst->work_index].cust_no)
        {
            last_cust_to_enter_flag = -1;
            sem_post(&machine_lock);
        }
        return NULL;
    }
    int ing_flag = ingreadient_checker(aglst->topping_stock, cust[aglst->work_index].cust_no, aglst->topping_list, aglst->t);

    if (ing_flag == 0 || ing_flag == -1)
    {
        printf(RED "Customer %d was rejected due to ingredient shortage so order was not accepted\n" RESET, cust[aglst->work_index].cust_no);
        cust[aglst->work_index].order_completion_status = 1;
        // totalorders = totalorders + cust[aglst->work_index].total_orders;
        sem_post(&superlock);
        if (last_cust_to_enter_flag == cust[aglst->work_index].cust_no)
        {
            last_cust_to_enter_flag = -1;
            sem_post(&machine_lock);
        }
        return NULL;
    }
    printf(YELLOW "Customer %d enters at time %d sec\n" RESET, aglst->work_index, tmr);
    printf(YELLOW "Customer %d orders %d ice creams\n" RESET, aglst->work_index, cust[aglst->work_index].total_orders);
    for (int i = 0; i < totalorders; i++)
    {
        if (order_list[i].cust_no == aglst->work_index)
        {
            printf(YELLOW "Ice cream %d : %s " RESET, order_list[i].order_no_cust, order_list[i].flavour);
            for (int j = 0; j < order_list[i].totaltoppings; j++)
                printf(YELLOW "%s " RESET, order_list[i].toppings[j]);
            printf("\n");
        }
    }
    current_customers++;
    // sem_wait(&superlock);
    // sem_post(&order_number);
    sem_post(&superlock);
    if (last_cust_to_enter_flag == cust[aglst->work_index].cust_no)
    {
        last_cust_to_enter_flag = -1;
        sem_post(&machine_lock);
    }
    sem_wait(&rej_customers[aglst->work_index]);
    if (close_sig == 1)
    {
        if(some_ing_exh==1)
        {
            printf(RED"Customer %d was not serviced because parlor closed suddenly\n"RESET,aglst->work_index);
            return NULL;
        }
        printf(RED "Customer %d was not serviced due to unavailability of machines\n" RESET, aglst->work_index);
        return NULL;
    }
    if (rejection_cust[aglst->work_index] == 1)
    {
        cust[aglst->work_index].order_completion_status = 1;
        printf(RED "Customer %d left at %d second(s) with an unfulfilled order\n" RESET, cust[aglst->work_index].cust_no, tmr);
        // sem_post(&capacity);
        current_customers--;
        return NULL;
    }
    sem_wait(&customers[aglst->work_index]);
    cust[aglst->work_index].order_completion_status = 1;
    printf(GREEN "Customer %d has collected their order(s) and left at %d second(s)\n" RESET, aglst->work_index, tmr);
    // sem_post(&capacity);
    current_customers--;
    return NULL;
}
int populate_order(char *com, int order_index, int cust)
{
    int i = 0;
    char **array = (char **)malloc(sizeof(char *) * 100);
    for (int i = 0; i < 100; i++)
        array[i] = (char *)malloc(sizeof(char) * 100);
    char t[100];
    char *token;
    char *ptr_out = NULL;
    char del[] = " \t\f\v\r\n";
    token = NULL;
    ptr_out = NULL;
    strcpy(t, com);
    token = __strtok_r(t, del, &ptr_out);
    i = 0;
    while (token != NULL)
    {
        strcpy(array[i++], token);
        token = __strtok_r(NULL, del, &ptr_out);
    }
    int total_count = i;
    if (order_index == 0)
    {
        order_list[order_index].cust_no = cust;
        strcpy(order_list[order_index].flavour, array[0]);
        order_list[order_index].order_no_cust = 1;
        order_list[order_index].order_taken_status = 0;
        order_list[order_index].orderstatus = 0;
        order_list[order_index].totaltoppings = total_count - 1;
        for (int j = 1; j < total_count; j++)
        {
            strcpy(order_list[order_index].toppings[j - 1], array[j]);
        }
    }
    else
    {
        order_list[order_index].cust_no = cust;
        strcpy(order_list[order_index].flavour, array[0]);
        if (order_list[order_index].cust_no != order_list[order_index - 1].cust_no)
            order_list[order_index].order_no_cust = 1;
        else
            order_list[order_index].order_no_cust = order_list[order_index - 1].order_no_cust + 1;
        order_list[order_index].order_taken_status = 0;
        order_list[order_index].orderstatus = 0;
        order_list[order_index].totaltoppings = total_count - 1;
        for (int j = 1; j < total_count; j++)
        {
            strcpy(order_list[order_index].toppings[j - 1], array[j]);
        }
    }
}
// int customer_incoming(int* cust_arrival)
// {

// }
int main()
{
    char *input1 = malloc(sizeof(char) * 512);
    init_ordernode();
    int n, k, f, t;
    fgets(input1, 512, stdin);
    sscanf(input1, "%d %d %d %d", &n, &k, &f, &t);
    int machines_remaining[n];
    int machine_start[n];
    int machine_end[n];
    int max = 0;
    for (int i = 0; i < n; i++)
    {
        fgets(input1, 512, stdin);
        sscanf(input1, "%d %d", &machine_start[i], &machine_end[i]);
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
    {
        fgets(input1, 512, stdin);
        sscanf(input1, "%s %d", flavour_list[i], &flavour_time[i]);
    }
    // for(int i=0;i<f;i++)
    // printf("%s\n",flavour_list[i]);
    for (int i = 0; i < t; i++)
    {
        fgets(input1, 512, stdin);
        sscanf(input1, "%s %d", topping_list[i], &topping_stock[i]);
    }
    char *input = (char *)malloc(sizeof(char) * 10);
    int cust_cnt = 0;
    int order_cnt = 0;
    // getchar();

    /*
    2 3 2 3
    0 7
    4 10
    vanilla 3
    chocolate 4
    caramel 1
    brownie 4
    strawberry 4
    1 1 2
    vanilla caramel
    chocolate brownie strawberry
    2 2 1
    vanilla strawberry caramel

    */

    while (fgets(input, 10, stdin) != NULL)
    {
        int cust_no;
        int cust_arrival_time;
        int t_orders;
        // printf("input:%s\n",input);
        sscanf(input, "%d %d %d", &cust_no, &cust_arrival_time, &t_orders);
        cust[cust_cnt + 1].cust_arrival = cust_arrival_time;
        cust[cust_cnt + 1].cust_no = cust_no;
        cust[cust_cnt + 1].order_completion_status = 0;
        cust[cust_cnt + 1].total_orders = t_orders;
        // printf("cust-no:%d\n",cust_no);
        // printf("t_orders:%d\n",t_orders);
        char *order = (char *)malloc(sizeof(char) * 1000);
        for (int i = 0; i < t_orders; i++)
        {
            fgets(order, 1000, stdin);
            // printf("order_index:%d\n",order_cnt);
            populate_order(order, order_cnt, cust_no);
            order_cnt++;
        }
        cust_cnt++;
        // getchar();
    }

    // for(int i=0;i<order_cnt;i++)
    // {
    //     printf("order %d flavour %s toppings:",i,order_list[i].flavour);
    //     for(int j=0;j<order_list[i].totaltoppings;j++)
    //     printf("%s ",order_list[i].toppings[j]);
    //     printf("\n");
    // }
    // printf("\n");
    // for(int i=0;i<cust_cnt;i++)
    // {
    //     printf("cust no %d cust arrival time %d customer orders %d\n",cust[i+1].cust_no,cust[i+1].cust_arrival,cust[i+1].total_orders);
    // }

    // int num_cust;
    // scanf("%d", &num_cust);
    // // printf("%d\n",num_cust);
    // int count = 0;
    // for (int i = 1; i <= num_cust; i++)
    // {
    //     int num_t;
    //     scanf("%d %d %d", &cust[i].cust_no, &cust[i].cust_arrival, &cust[i].total_orders);
    //     cust[i].order_completion_status = 0;
    //     for (int j = 0; j < cust[i].total_orders; j++)
    //     {
    //         scanf("%d", &num_t);
    //         order_list[count].cust_no = cust[i].cust_no;
    //         order_list[count].totaltoppings=num_t;
    //         scanf("%s", order_list[count].flavour);
    //         for (int k = 0; k < num_t; k++)
    //             scanf("%s", order_list[count].toppings[k]);
    //         count++;
    //     }
    // }
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
    pthread_t customer_threads[cust_cnt];
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
    struct arglist arg2[cust_cnt + 1];
    for (int i = 1; i <= cust_cnt; i++)
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
        pthread_create(&customer_threads[i-1], NULL, customer_waiting, &arg2[i]);
    }

    // int returned_count = 0;
    while (tmr <= max && !(ingredient_exhausted_checker(topping_stock, t) && any_order_being_prepared(order_cnt)))
    {   printf(BMAG"time: %d sec\n\n"RESET,tmr);
        int total_about_to_enter = 0;
        int total_about_to_enter2 = 0;
        sem_wait(&machine_lock);
        for (int i = 1; i <= cust_cnt; i++)
        {
            if (cust[i].cust_arrival == tmr)
                total_about_to_enter++;
        }
        total_about_to_enter2 = total_about_to_enter;
        for (int i = 1; i <= cust_cnt; i++)
        {
            if (cust[i].cust_arrival == tmr)
            {
                if (total_about_to_enter == 1)
                {
                    last_cust_to_enter_flag = cust[i].cust_no;
                }
                sem_post(&customers[i]);
                total_about_to_enter--;
            }
        }
        if (total_about_to_enter2 == 0)
            sem_post(&machine_lock);
        for (int i = 0; i < n; i++)
        {
            if (machine_start[i] == tmr)
            {
                sem_post(&machines[i]);
            }
        }
        for (int i = 0; i < n; i++)
        {
            if (machine_start[i] <= tmr && tmr <= machine_end[i] && (finish_time_order[i] < tmr || special_flag[i] == 1))
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
                printf(ORANGE "Machine no %d stopped at %d sec\n" RESET, i + 1, tmr);
            }
        }
        tmr += 1;
        printf("\n");
    }
    printf(BMAG"time: %d sec\n\n"RESET,tmr);
    if ((ingredient_exhausted_checker(topping_stock, t) && any_order_being_prepared(order_cnt)) && tmr < max)
    {
        some_ing_exh = 1;
    }
    close_sig = 1;
    for (int i = 0; i < n; i++)
        sem_post(&machines[i]);
    for (int i = 1; i <= cust_cnt; i++)
    {
        if (cust[i].order_completion_status == 0)
        {
            sem_post(&rej_customers[i]);
        }
    }
    for(int i=0;i<n;i++)
    pthread_join(machine_threads[i],NULL);
    for(int i=1;i<=cust_cnt;i++)
    pthread_join(customer_threads[i-1],NULL);
    if(some_ing_exh==0)
    printf(WHITE "Parlour closed\n" RESET);
    if(some_ing_exh==1)
    printf(RED"Parlour closed because some ingredient exhausted\n"RESET);
    printf(RED "No. of customers left due to space unavailability: %d \n" RESET, returned_count);
}

/*

2 3 2 3
0 7
4 10
vanilla 3
chocolate 4
caramel 1
brownie 4
strawberry 4
1 1 2
vanilla caramel
chocolate brownie strawberry
2 2 1
vanilla strawberry caramel

*/

/*

2 3 3 3
0 8
4 10
vanilla 3
chocolate 4
sid 2
caramel 1
brownie 4
strawberry 4
1 1 2
vanilla caramel
chocolate brownie strawberry
2 2 1
sid strawberry caramel

*/

/*

2 1000 2 3
0 7
4 10
vanilla 3
chocolate 4
caramel -1
brownie 4
strawberry 4
1 1 2
vanilla caramel
chocolate brownie strawberry
2 2 1
vanilla strawberry caramel
3 8 1
vanilla caramel
4 9 1
vanilla caramel
5 13 1
vanilla caramel
6 15 1
vanilla caramel

*/

/*

1 10 1 1
0 5
vanilla 1
caramel 1
1 1 1
vanilla caramel
2 2 1
vanilla caramel

*/

/*

1 10 1 1
0 5
vanilla 1
caramel 2
1 1 1
vanilla caramel
2 2 1
vanilla caramel


*/

/*

1 1 2 3
0 18
vanilla 3
chocolate 4
caramel -1
brownie 4
strawberry 4
1 1 2
vanilla caramel
chocolate brownie strawberry
2 2 1
vanilla strawberry caramel


*/

/*

1 1 2 3
0 10
vanilla 3
chocolate 4
caramel -1
brownie 4
strawberry 4
1 1 2
vanilla caramel
chocolate brownie strawberry

*/

/*

1 1 2 3
0 10
vanilla 3
chocolate 4
caramel -1
brownie 4
strawberry 4
1 1 1
vanilla caramel
2 1 1
chocolate brownie strawberry

*/

/*

2 1 1 1
0 5
0 6
vanilla 2
caramel -1
1 1 1
vanilla caramel

*/

/*

2 4 3 5
0 20
1 22
A 3
B 4
C 5
t1 2
t2 1
t3 2
t4 -1
t5 -1
1 1 2
A t1 t2
B t1 t3
2 2 1
A t3 t2
3 2 1
A t3 t5
4 3 1
A t4 t5
5 3 1
B t4 t5

*/