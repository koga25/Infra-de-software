#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define LIMIT 20000000
#define TRUE 1
#define FALSE 0

//functions
void* IncreaseCounter();

//global variables

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long long counter = 0;
unsigned char printCounter = TRUE;


int main()
{
    //initializing counter
    pthread_t* newThread;
    printf("how many threads do you want? (0 < n <= 255)\n");

    //thread limit is 255, 0 is not accepted. overflows if higher than 255.
    unsigned char threadCount = 0;
    while(threadCount == 0)
    {
        scanf("%hhu", &threadCount);
        printf("%hhu threads chosen\n", threadCount);
    }

    //allocating n threads
    newThread = (pthread_t*) malloc (sizeof(pthread_t) * threadCount);
    for (int i = 0; i < threadCount; i++)
        pthread_create(&newThread[i], NULL, IncreaseCounter, &newThread);
    

    //wait for all threads to finish.
    for (int i = 0; i < threadCount; i++)
        pthread_join(newThread[i], NULL);


    printf("exiting program\n");
    printf("%lld\n", counter);
    free(newThread);
    return 0;
}


void* IncreaseCounter( void* arg) 
{
    //locking this part of the code to make only one thread able to acess at any time
    pthread_mutex_lock(&mutex);
    while(counter < LIMIT)
    {
        counter++;
        //if counter hits the LIMIT, print the counter and finish all threads.
        
        if(counter == LIMIT)
        {
            
            printf("%lld\n", counter);
        }    
    }
    //unlocking
    pthread_mutex_unlock(&mutex); 
    pthread_exit(NULL);
    

    return NULL;
}