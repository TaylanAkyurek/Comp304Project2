# Comp304Project2
spacecraft control with pthreads and mutexes

Ali Taylan Akyürek									64229
Ardıl Deniz Kıratlı									68262

PROJECT 2 REPORT
Part 1
We first constructed queues and threads for LandingJob, AssemblyJob, LaunchJob and also for control towers. We came up with a solution for control tower by making two separate control towers for each pad. Control tower A checks and acknowledgements pad A and control tower B checks and acknowledgements pad B. Given jobs such as landing, launch and assembly are enqued at Job functions and dequed in control towers and handled based on their priorities which means control towers are determines the timing. Moreover, precise usage of mutexes in control towers blocks race condition, without a mutex there will be data race. We use mutex everytime we want to access same shared data. We use mutexes when enqueueing, dequeueing, checking if its empty etc. It continuous until simulation ends. LogWrite writes the job in log.txt file with its ID, status and timings and pad. PrintQueues prints the IDs of the rockets.
 

while(!isEmptyLanding(landingQueue){ // To make landing favored for part 1. 
			…
}

Part 2

Since solution in part 1 causes starvation 3 jobs in waiting and with enough more jobs coming to control towers, they can not take another landing jobs so landing loses its priority. We came up with a solution that checks assemblyQueue size is smaller than 3 or launchQueue size is smaller than 3 and it solves the starvation with aging by checking whether current time minus age is surpassed the maximum waiting time or not. So,  aging guarantees that even if while loop  does not work after a certain time, it works after 20 seconds. It solves the starvation in the orbit.

For part 2, to achieve statement:

The control tower favors the landing pieces and lets the waiting pieces in the orbit to land until one of following conditions hold 

• (a) No more pieces are waiting to land,
 • (b) 3 or more launches or 3+ assemblies are waiting to be scheduled on the ground.

We changed our code as:

 while(!isEmptyLanding(landingQueue) && assemblyQueue -> size < 3){

}

That takes the priority from landing queue if there is more than 3 assembly or launch job. But now, this may cause landing jobs to starvation because some jobs take too long (such as assembly takes 12 seconds) and there may be always more than 3 jobs in queue if given p is big. To prevent this, we add an aging mechanism and a maximumWaitTime. Because of this modification, if there was not a landing job for 20 seconds, priority will be given to landing.

Modified code:

int age = time(NULL);

while(!isEmptyLanding(landingQueue) && (assemblyQueue -> size < 3 || time(NULL) - age > maximumWaitTime){

					…

int age = time(NULL); // to reset age if condition above holds.
}



Part 3

We againg constructed queue and thread for emergencyJob and control towers checks first emergency queue instead of regular landings and it creates two jobs for towers instead of one. It checks the Boolean value which turns true after a certain time.  And priority is always in emergency because of the modification:

 while((!isEmptyLanding(landingQueue) && (assemblyQueue -> size < 3 || time(NULL) - age > maximumWaitTime)) && isEmptyEmergency(emergencyQueue)){

					…
}

isEmptyEmergency(emergencyQueue) is exists at every priority check. So a job other than emergency can be done if and only if emergency queue is empty.
![image](https://user-images.githubusercontent.com/79318145/167126590-394196fa-e864-4636-a3d4-4065249da8f4.png)
