[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/JH3nieSp)
# OSN Monsoon 2023 mini project 3


# xv6 revisited

## PBS REPORT

### Analysis:

#### RBI Analysis:
  According to given conditions Rtime will take only two values 0 or 1.
So , let’s take both conditions to analyse value of RBI.

 $RBI = max(Int(\frac{((3*Rtime - Stime - Wtime)*50)}{(Rtime+Wtime+Stime+1)}),0)$
 1. Rtime is 0:  Since we know that Stime and Wtime both will be non negative. So,
   we can see in formula that when Rtime will be 0 then $(3*Rtime - Stime - Wtime)*50$ will be negative when Stime and Wtime both values will be greater than 0 and int $(3*Rtime - Stime - Wtime)*50$ will be 0 when Wtime and Stime both values will be 0.
   Hence , the RBI value will be max(-ve,0) or max(0,0). So RBI will be 0 if Rtime is 0.
 2. Rtime is 1:  When Rtime will be 1 then we will get $RBI=max(Int(\frac{((3 - (Stime + Wtime))*50)}{(2+Wtime+Stime)}),0)$
    Here we can see that if $0<= Stime + Wtime <=2 $ then RBI will be 1 because $\frac{((3 - (Stime + Wtime))*50)}{(2+Wtime+Stime)}$ will lie between [12, 75] . So $RBI will belong to [12,75]$.
    If $Stime + Wtime >= 3 $ then we can see that $\frac{((3 - (Stime + Wtime))*50)}{(2+Wtime+Stime)} < 1$ will be either 0 or negative. So in both of these cases $RBI =0$

    hence here we saw that RBI will be some positive value only when $0 <= Wtime + Stime <=2$ otherwise $RBI=0$

    Since, a process will have $0 <= Wtime + Stime <=2$ for a very short period of time , so RBI will be equal to 0 for most of the time.

    In both conditions of $Rtime = 0$ and $Rtime = 1$, RBI is equal to 0 (uneffective) most of the times.
    So, RBI will have negligible(for very short period of time) impact on deciding the Scheduling.Most of the the times it will be 0.

#### SP Analysis:
   Since above we saw that RBI won't have much impact on scheduling (It will be 0 for most of the times), so SP will be the DP value (Most of the times) , which will decide scheduling process. 
   $DP = min(SP+RBI, 100)$

### Implementation:
   - First add running_time, wait_time , sleep_time, schedule_count & rbi to the proc struct in proc.h
   - Initialize them in alloc proc function in proc.c
   - Update running_time, wait_time and sleep_time in update_time function in proc.c which gets called whenever clock interrupt is raised.
   - Whenever a process is RUNNING , then it's sleep_time will be 0, and running_time will get incremented.
   - Whenever a process is WAITING , then it's wait_time will get incremented.
   - Whenever a process is sleeping, then it's sleep_time will get incremented.
   - In Scheduler function loop through the list of all processes to choose a RUNNABLE process with minimum DP value, which will be calculated using the given formula for DP and RBI.Once a process is chosen , then run it.
   - If DP of two processes is equal then tie will be broken based on the schedule count of each processes, the process with lower schedule count will get chance to get scheduled and will be chosen. (Fairness)
   - If DP and Schedule count both are equal for some processes then the Start time of processes will be considered and the process who started first will get chance to get scheduled. ( start time of process is stored as p->ctime).
   - Write down set_priority function which will take two arguments pid and priority. It changes the static priority of the pid process to the new priority given in argument. It resets the rbi to 25 and if new priority of process is higher than old one (means static priority value is lower than previous one), then yield() to give control to cpu to schedule again.

    
# Concurrency:

## Cafe Sim:

### Implementation:
   - For each Customer and Barista a new thread is created which will simulate Baristas and Customers.
   - A clock is maintained in main thread using sleep which will keep track of each passing second.
   - A Customer thread starts executing itself the moment when customer corresponding to the thread enters in cafe.
   - In customer thread Customer gives order and waits for it's order to get prepared.
   - In Barista thread , each non-occupied Barista will go through order list and pick a order if the order has not been picked to prepare yet.
   - Barista thread then prepares order and notifies Customer thread when order is fully prepared.
   - Customer thread will wait till tol_time of corresponding customer and then it will leave without order.
   - Whenever a Customer leaves without order and Barista prepares the coffee, then that coffee is wasted and wasted_coffee is incremented.(maintain variables for wasted_coffee and wait_time)
   - There might be a case when Customer leaves before it's order gets started to be prepared. In this case the Barista won't prepare order to minimize waste_coffee count.
   - Since Barista threads and Customer threads will run concurrently , so semaphors are used to give atomicity to critical sections of code.

### Questions:
 1. Wating time:
   wait_time is incremented in two ways for two different scenarios. When a customer gets it's order then the wait time is equal to $(leaving_time - arrival_time - preparation_time_of_coffee)$ . Preparation time of coffee is not added in wait_time if a customer gets served. While if a customer leaves without their order, then wait_time is equal to $(tol_time_of_customer + 1)$.
   If the cafe had infinite baristas then the wait_time would be $1$ sec for every customer who got served and $(tol_time +1)$
   for all those customers who didn't get served (because they had less tol_time than the prep_time of cofee).

   While in the given scenario the wait_time for a customer who did get served will be $>=1$ and for then who didn't get served the wait_time would be same as infinite barista case $(tol_time + 1)$.

   So in the given finite baristas scenario the wait_time will either be equal to the case of inifinite baristas , this will happen if each customer gets assigned a barista at the very  moment of it's arrival. Or the wait_time will be greater than the case of infinite baristas (if baristas are busy , when customer arrived).
   So , generally the average wait_time in given finite baristas scenario will be greater than inifinite baristas scenario.

 2. Coffee Wastage:
   Coffee gets wasted when a barista starts preparing coffee and the corresponding customer leaves without coffee before the coffee is prepared. To avoid this we can implement functionality of checking that whether the coffee can be prepared before tolerance exceeding time of customer or not, if coffee can't be prepared before that then the barista will not start preparing the coffee, knowing that anyhow it is going to get wasted if prepared. So this will minimize waste_coffee count to 0.

## Ice Cream Parlor Sim

### Implementation:
   - For each Customer and Machine a new thread is created which will simulate Machines and Customers.
   - A clock is maintained in main thread using sleep which will keep track of each passing second.
   - A Customer thread starts executing itself the moment when customer corresponding to the thread enters in cafe.
   - Similarly a machine thread starts executing itself the moment when it's start time is arrived.
   - In customer thread Customer gives order and waits for it's order to get prepared.
   - In Machine thread , each non-occupied Machine will go through order list and pick an ice cream if the ice cream has not been picked to prepare yet,if the required ingredients for that particular ice cream is sufficient and if the order can be prepared before the stop time of machine.
   - If Machine finds out that any ingredient required for any particular ice cream is insufficient then it will reject whole order of that particular corresponding customer at that moment.
   - Machine thread then prepares order and notifies Customer thread when order is fully prepared.
   - Whenever a Customer leaves without order and Machine prepares the partial order, then that partial order is wasted.
   - If all ingredients which were limited get exhausted then the shop is closed (though if some machine is still preparing some order, then it will wait for all machines to get free and then shop will get closed).
   - Since Barista threads and Machine threads will run concurrently , so semaphors are used to give atomicity to critical sections of code.
   - If a Machine starts a t sec and find out that some order is waiting from before time then it will start preparing that ice cream immediately.
   - If a customer leaves then it's place will be empty from next second only.
   - If shop is full (capacity is reached), then incoming customers won't wait and they will return immediately.

### Questions:
1.Minimizing Incomplete Orders:
  To minimize incomplete orders I have implemented my machine in such way that it will prepare order as much as possible. Lets say machine finds out that it won't be able to prepare that particular order because it doesn't have much time to prepare that , so it won't pick that order but instead it will check next orders to find out some order which can be prepared in the available time before the machine gets closed. This way every machine will try to make maximum number of ice creams in their shift.
  To minimize this situation further, we can implement a functionality of precomputing the time for preparation of that particular order and assigning ingredients and machines on spot , so that it is guaranteed that the customer will get served for sure. This way the orders will only get accepted if it can be prepared for sure.

2.Ingredient Replenishment:
  If the ingredients can be replenished then whenever a Machine will find out that ingredient is depleted then it will check if the time required for replenishment + time required to prepare order is less than the available time or not. If the Machine is about to get closed before the replenishment process completes then the machine will ignore that order for a moment( it won't reject it because their is a possibility that another upcoming machine can afford the preparation time and replenishment time of that order), and if the time required for replenishment + time required to prepare order is less than the available time then the machine will call replenishment and then will prepare the order. 
  
  We can also use replenishment in some other way. Just keep a limit, below which if some ingredient amount goes then replenishment for that ingredient is called immediately. One extra replenishment machine thread is made (which will keep checking the amount of ingredients and will perform replenishment).

3.Unserviced Orders: 
  We can do precomputation of time slots required for each order and we can assign machine time slots priorily to each order. By this, machines will prepare order in predetermined way and no order will get accepted if it can't be prepared by any machine about to come or available, or if no more required slots of any machine is available. This way we can reduce the number of Unserviced Orders.