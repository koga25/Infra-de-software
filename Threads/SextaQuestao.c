#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXBUFFERSIZE 5
#define CONSUMERSIZE 2
#define PRODUCERSIZE 2

typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
}BlockingQueue;

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
BlockingQueue* blockQueue;

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer);
void* consumer(void* arg);
void* producer(void* arg);
void putBlockingQueue(BlockingQueue* Q,int newValue);
int takeBlockingQueue(BlockingQueue* Q);

int main(int argc, char const *argv[])
{
   sem_init(&empty, 1, MAXBUFFERSIZE);
   sem_init(&full, 1, 0);
   blockQueue = newBlockingQueue(MAXBUFFERSIZE);

   pthread_t* consumers = (pthread_t*)malloc(sizeof(pthread_t)*CONSUMERSIZE);
   pthread_t* producers = (pthread_t*)malloc(sizeof(pthread_t)*PRODUCERSIZE);

   for(int i = 0; i < CONSUMERSIZE; i++)
   {
      u_int8_t* position = (u_int8_t*)malloc(sizeof(u_int8_t));
      position[0] = i;
      pthread_create(&consumers[i], NULL, consumer, position);
   }

   for(int i = 0; i < PRODUCERSIZE; i++)
   {
      u_int8_t* position = (u_int8_t*)malloc(sizeof(u_int8_t));
      position[0] = i;
      pthread_create(&producers[i], NULL, producer, position);
   }

   for(int i = 0; i < CONSUMERSIZE; i++)
   {
      pthread_join(consumers[i], NULL);
   }

   for(int i = 0; i < PRODUCERSIZE; i++)
   {
      pthread_join(producers[i], NULL);
   }

   return 0;
}

BlockingQueue* newBlockingQueue(unsigned int SizeBuffer)
{
   BlockingQueue* blockQueue = (BlockingQueue*)malloc(sizeof(BlockingQueue));
   blockQueue->head = NULL;
   blockQueue->last = NULL;
   blockQueue->sizeBuffer = SizeBuffer;
   blockQueue->statusBuffer = 0;
   return blockQueue;
}

void* consumer(void* arg)
{
   u_int8_t* position = (u_int8_t*)arg;
   u_int8_t value = 0;
   while(1)
   {
      if(blockQueue->statusBuffer == 0)
      {
         printf("Consumer [%i] entering sleep, node quantity [%i]\n", position[0],blockQueue->statusBuffer);
         
      }
      sem_wait(&empty);
      if(blockQueue->statusBuffer < 0)
         exit(1);

      printf("Consumer[%i] initiating putBlockingQueue, node quantity [%i]\n", position[0], blockQueue->statusBuffer);
      value = rand();
      putBlockingQueue(blockQueue, value);
      sem_post(&full);
   }
}

void* producer(void* arg)
{
   u_int8_t* position = (u_int8_t*)arg;
   while(1)
   {
      if(blockQueue->statusBuffer >= MAXBUFFERSIZE)
      {
         printf("Producer [%i] entering sleep node quantity [%i]\n", position[0], blockQueue->statusBuffer);
         
      }  
      sem_wait(&full);
      if(blockQueue->statusBuffer > MAXBUFFERSIZE)
         exit(1);
      
      printf("Producer[%i] initiating putBlockingQueue, node quantity [%i]\n", position[0], blockQueue->statusBuffer);
      takeBlockingQueue(blockQueue);
      sem_post(&empty);
   }
}

void putBlockingQueue(BlockingQueue* Q, int newValue)
{
   Elem* elem = (Elem*)malloc(sizeof(Elem));
   elem->value = newValue;
   elem->prox = NULL;
   //seção crítica
   pthread_mutex_lock(&mutex);
   if(Q->head== NULL)
   {
      Q->last = elem;
      Q->head = Q->last;
   }
   else
   {
      Q->last->prox = elem;
      Q->last = Q->last->prox;
   }
   Q->statusBuffer++;
   pthread_mutex_unlock(&mutex);
}

int takeBlockingQueue(BlockingQueue* Q)
{
   pthread_mutex_lock(&mutex);
   Elem* elem = Q->head;
   Q->head = Q->head->prox;
   Q->statusBuffer--;
   pthread_mutex_unlock(&mutex);
   free(elem);
}
