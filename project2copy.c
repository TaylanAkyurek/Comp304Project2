#include "queue.c"



int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 40; // frequency of emergency
float p = 0.2;               // probability of a ground job (launch & assembly)
int n = 30;

bool isStarted = true;
bool queuePrinted = false;
int launchNext = 101;
int assemblyNext = 201;
int landingNext = 301;
int emergencyNext = 401;

int startTime;
time_t start;
struct Queue *launchQueue; 
struct Queue *assemblyQueue; 
struct Queue *landingQueue; 
struct Queue *emergencyQueue; 


struct Queue *padA; 
struct Queue *padB;




pthread_mutex_t launchMutex;
pthread_mutex_t assemblyMutex;
pthread_mutex_t landingMutex;
pthread_mutex_t emergencyMutex;

pthread_mutex_t padAMutex;
pthread_mutex_t padBMutex;

pthread_mutex_t controlMutex;

pthread_mutex_t logFile;

pthread_mutex_t launchControlMutex;
pthread_mutex_t assemblyControlMutex;
pthread_mutex_t landingControlMutex;
pthread_mutex_t emergencyControlMutex;

pthread_mutex_t emptyControlMutex;

pthread_cond_t launchCond;
pthread_cond_t assemblyCond;
pthread_cond_t landingCond;
pthread_cond_t emergencyCond;



void* LandingJob(void *arg); 
void* LaunchJob(void *arg);
void* EmergencyJob(void *arg); 
void* AssemblyJob(void *arg); 
void* ControlTowerA(void *arg); 
void* ControlTowerB(void *arg); 
void* logWrite(char* a);
void* printQueues();
void* printQueue(Queue *q);
bool isEmptyLanding(Queue *q);
bool isEmptyEmergency(Queue *q);

// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock (&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock (&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}

int main(int argc,char **argv){
    startTime = time(NULL);
    // -p (float) => sets p
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-p")) {p = atof(argv[++i]);}
        else if(!strcmp(argv[i], "-t")) {simulationTime = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-s"))  {seed = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-n"))  {n = atoi(argv[++i]);}

    }
    
    srand(seed); // feed the seed
    
    /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Job j;
        j.ID = myID;
        j.type = 2;
        Enqueue(myQ, j);
        Job ret = Dequeue(myQ);
        DestructQueue(myQ);
    */

    // your code goes here
    start = time(NULL);
    


    launchQueue = ConstructQueue(100);

    pthread_mutex_init(&launchControlMutex, NULL);
    pthread_mutex_init(&launchMutex, NULL);
    
    landingQueue = ConstructQueue(100);

    pthread_mutex_init(&landingControlMutex, NULL);
    pthread_mutex_init(&landingMutex, NULL);

    assemblyQueue = ConstructQueue(100);

    pthread_mutex_init(&assemblyControlMutex, NULL);
    pthread_mutex_init(&assemblyMutex, NULL);

    emergencyQueue = ConstructQueue(100);

    pthread_mutex_init(&emergencyControlMutex, NULL);
    pthread_mutex_init(&emergencyMutex, NULL);

    pthread_mutex_init(&padAMutex, NULL);
    pthread_mutex_init(&padBMutex, NULL);

    pthread_mutex_init(&logFile, NULL);



    pthread_t controlA;
    
    pthread_create(&controlA, NULL, ControlTowerA, NULL);

    pthread_t controlB;
    
    pthread_create(&controlB, NULL, ControlTowerB, NULL);

    bool e = false;

    pthread_t initialTakeOff;

    pthread_create(&initialTakeOff, NULL, LaunchJob, NULL);

while (time(NULL) - startTime < simulationTime){
    

    if(time(NULL) - startTime > n){


        printQueues();
    }

    double random = (rand() % 10001) / 10000.0;

    if((time(NULL) - startTime) %40 == 0 && (time(NULL) - startTime) != 0 ){

        pthread_t emergency;
        e = true;
        pthread_create(&emergency, NULL, EmergencyJob, (void*) e);

    }
    if(random <= p/2){
    pthread_t launch;
    pthread_create(&launch, NULL, LaunchJob, NULL);

    }
    random = (rand() % 10001) / 10000.0;

    if(random < (1-p)){
    pthread_t landing;
    pthread_create(&landing, NULL, LandingJob, NULL);
    }
   
    random = (rand() % 10001) / 10000.0;
    
    if(random <= p/2){
    pthread_t assembly;
    pthread_create(&assembly, NULL, AssemblyJob, NULL);
    }

    pthread_sleep(2);

    }
    return 0;
}

// the function that creates plane threads for landing
void* LandingJob(void *arg){
    Job *rocket = malloc(sizeof(*rocket));


    pthread_mutex_lock(&landingMutex);   

    
    rocket -> ID = landingNext;
    landingNext += 1;
    rocket -> status = 'L';
    rocket -> requestTime = time(NULL)- startTime;

  //  printf("id %d  status %c  request time %d \n",rocket -> ID, rocket-> status, rocket -> requestTime );

    Enqueue(landingQueue, *rocket);

    pthread_mutex_unlock(&landingMutex);


}

// the function that creates plane threads for departure
void* LaunchJob(void *arg){
    
    Job *rocket = malloc(sizeof(*rocket));



    pthread_mutex_lock(&launchMutex);
    
    rocket -> ID = launchNext;

    launchNext += 1;
    rocket -> status = 'D';
    rocket -> requestTime = time(NULL) - startTime;

  //  printf("id %d  status %c  request time %d \n",rocket -> ID, rocket-> status, rocket -> requestTime );

    Enqueue(launchQueue, *rocket);
  
    pthread_mutex_unlock(&launchMutex);

}

// the function that creates plane threads for emergency landing
void* EmergencyJob(void *arg){

    Job *rocket1 =malloc(sizeof(*rocket1));
    Job *rocket2 = malloc(sizeof(*rocket2));

    bool emergency = (bool*) arg;
    if(emergency){
        pthread_mutex_lock(&emergencyMutex);
        rocket1 -> ID = emergencyNext;

        emergencyNext += 1;
        rocket1 -> status = 'E';
        rocket1 -> requestTime = time(NULL) - startTime;

    //    printf("id %d  status %c  request time %d \n",rocket1 -> ID, rocket1-> status, rocket1 -> requestTime );

        Enqueue(emergencyQueue, *rocket1);
  
        rocket2 -> ID = emergencyNext;

        emergencyNext += 1;
        rocket2 -> status = 'E';
        rocket2 -> requestTime = time(NULL) - startTime;

    //    printf("id %d  status %c  request time %d \n",rocket2 -> ID, rocket2-> status, rocket2 -> requestTime );

        Enqueue(emergencyQueue, *rocket2);

        pthread_mutex_unlock(&emergencyMutex);

    }

    
}

// the function that creates plane threads for emergency landing
void* AssemblyJob(void *arg){
    
    Job *rocket = malloc(sizeof(*rocket));



    pthread_mutex_lock(&assemblyMutex);   

    
    rocket -> ID = assemblyNext;
    assemblyNext += 1;
    rocket -> status = 'A';
    rocket -> requestTime = time(NULL) - startTime;

  //  printf("id %d  status %c  request time %d \n",rocket -> ID, rocket-> status, rocket -> requestTime );

    Enqueue(assemblyQueue, *rocket);

    pthread_mutex_unlock(&assemblyMutex);   


}

// the function that controls the air traffic
void* ControlTowerA(void *arg){
  
    int age = time(NULL);
    int maximumWaitTime = 20;

    while(time(NULL)- startTime< simulationTime){
      
        bool unlockLand = true;
        bool unlockLaunch = true;
        bool unlockEmergency = true;

        pthread_mutex_lock(&padAMutex);

        while(!isEmptyEmergency(emergencyQueue)){


            pthread_sleep(2);

            if(!isEmptyEmergency(emergencyQueue)){
               
                pthread_mutex_lock(&emergencyControlMutex);
                
                Job j = Dequeue(emergencyQueue);
            
            //    printf("in pad A, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL) - startTime);
                    
                
                time_t end_time = time(NULL) - start;
                char log[100];
                sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime,time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'A');
                logWrite(log);

                pthread_mutex_unlock(&emergencyControlMutex);
                unlockEmergency = false;

            }
        }



        
        pthread_mutex_unlock(&padAMutex);

        pthread_mutex_lock(&padAMutex);


        while((!isEmptyLanding(landingQueue) && (launchQueue->size < 3 || time(NULL) - age > maximumWaitTime))&& isEmptyEmergency(emergencyQueue)){
            
            pthread_sleep(2);

            if(!isEmptyLanding(landingQueue)){
              
              
                pthread_mutex_lock(&landingControlMutex);


                Job j = Dequeue(landingQueue);

            //    printf("in pad A, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL) - startTime);
            

                char log[100];
                sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime, time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'A');
                logWrite(log);
                
                age = time(NULL);

                pthread_mutex_unlock(&landingControlMutex);
                unlockLand = false;
            }

            }

        pthread_mutex_unlock(&padAMutex);

        pthread_mutex_lock(&launchControlMutex);
        pthread_mutex_lock(&padAMutex);



        if((!isEmpty(launchQueue)) && isEmptyEmergency(emergencyQueue)){
            
           

            pthread_sleep(4);

            Job j = Dequeue(launchQueue);

        //    printf("in pad A, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL)- startTime);

            
            char log[100];
            sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime, time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'A');
            logWrite(log);
        }
        
        pthread_mutex_unlock(&launchControlMutex);
        pthread_mutex_unlock(&padAMutex);


    }

}
void* ControlTowerB(void *arg){

    int age = time(NULL);
    int maximumWaitTime = 20;
    while(time(NULL)- startTime < simulationTime){
          
        bool unlockLand = true;
        bool unlockAssembly = true;
        bool unlockEmergency = true;


        pthread_mutex_lock(&padBMutex);

        while(!isEmptyEmergency(emergencyQueue)){

            pthread_sleep(2);


            if(!isEmptyEmergency(emergencyQueue)){
                pthread_mutex_lock(&emergencyControlMutex);

                Job j = Dequeue(emergencyQueue);
            //   printf("in pad B, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL) - startTime);
                time_t end_time = time(NULL) - start;
                char log[100];
                sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime,time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'B');
                logWrite(log);
                pthread_mutex_unlock(&emergencyControlMutex);
                unlockEmergency = false;
                }
        }
        pthread_mutex_unlock(&padBMutex);

        pthread_mutex_lock(&padBMutex);

        while((!isEmptyLanding(landingQueue) && (assemblyQueue -> size < 3 || time(NULL) - age > maximumWaitTime)) && isEmptyEmergency(emergencyQueue)){
            
            pthread_sleep(2);

            if(!isEmptyLanding(landingQueue)){

                pthread_mutex_lock(&landingControlMutex);

            
                Job j = Dequeue(landingQueue);

            //    printf("in pad B, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL) - startTime);
                
                time_t end_time = time(NULL) - start;
                char log[100];
                sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime,time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'B');
                logWrite(log);
            
                age = time(NULL);

                pthread_mutex_unlock(&landingControlMutex);
                unlockLand = false;
            }
        }

        pthread_mutex_unlock(&padBMutex);

        pthread_mutex_lock(&assemblyControlMutex);
        pthread_mutex_lock(&padBMutex);

        if((!isEmpty(assemblyQueue)) && isEmptyEmergency(emergencyQueue)){
            

            pthread_sleep(12);
           

            Job j = Dequeue(assemblyQueue);

        //    printf("in pad B, end time for job %d with status %c = %d\n",j.ID, j.status ,time(NULL) - startTime);

            char log[100];
            sprintf(log, "%-5d %5c %10d %10d %10d %10c\n", j.ID, j.status, j.requestTime,time(NULL) - startTime, time(NULL) - startTime - j.requestTime, 'B');
            logWrite(log);
        }

        pthread_mutex_unlock(&assemblyControlMutex);
        pthread_mutex_unlock(&padBMutex);




    }
}
    void* logWrite(char* a){

        pthread_mutex_lock(&logFile);
        FILE* f;
        if(isStarted){
        f = fopen("log.txt", "w");
        fprintf(f, "EventID Status RequestTime EndTime  TurnaroundTime   Pad \n");
        isStarted = false;
        }
        else{
       
        f = fopen("log.txt", "a");


        }
        fprintf(f, "%s",a);
        fclose(f);
        pthread_mutex_unlock(&logFile);




    }

    void* printQueues(){

            pthread_mutex_lock(&landingMutex);
            printf("At %d sec landing: ", time(NULL) - startTime);
            printQueue(landingQueue);
            pthread_mutex_unlock(&landingMutex);

            pthread_mutex_lock(&launchMutex);
            printf("At %d sec launch: ", time(NULL) - startTime);
            printQueue(launchQueue);
            pthread_mutex_unlock(&launchMutex);

            pthread_mutex_lock(&assemblyMutex);
            printf("At %d sec assembly: ", time(NULL) - startTime);
            printQueue(assemblyQueue);
            pthread_mutex_unlock(&assemblyMutex);

            pthread_mutex_lock(&emergencyMutex);
            printf("At %d sec emergency: ", time(NULL) - startTime);
            printQueue(emergencyQueue);
            pthread_mutex_unlock(&emergencyMutex);

    }
    void* printQueue(Queue* q){
        if(isEmpty(q)){
            printf("empty\n");
        }
        else{
          NODE* current = q->head;
          while(current != NULL){
            printf("%d ", current->data.ID);
            current = current->prev;
         }
        printf("\n");
        }
}
    bool isEmptyLanding(Queue *q){

        bool result;
        pthread_mutex_lock(&landingControlMutex);

        if(isEmpty(q)){
            result = true;

        }
        else{

            result = false;
        }

        pthread_mutex_unlock(&landingControlMutex);

        return result;



    }
    bool isEmptyEmergency(Queue *q){

        bool result;
        pthread_mutex_lock(&emergencyControlMutex);

        if(isEmpty(q)){
            result = true;

        }
        else{

            result = false;
        }

        pthread_mutex_unlock(&emergencyControlMutex);

        return result;



    }
