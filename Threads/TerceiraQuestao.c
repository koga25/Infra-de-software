#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define TRUE 1
#define FALSE 0


long long counter = 0;
char string[810];
char subString[30];
unsigned short stringLength;
unsigned short subStringLength;
unsigned short currentPosition = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* Substrings(void* arg);
int GetThreadQuantity(char* string, char* subString);

int main()
{
    //initializing counter
    pthread_t* newThread;
    
    unsigned short threadQuantity;

    //getting the strings
    printf("type your string (0 < n <= 800)\n");    
    fgets(string, sizeof(string), stdin);   
    printf("type your subString (0 < n <= 20)\n");
    fgets(subString, sizeof(subString), stdin);

    threadQuantity = GetThreadQuantity(string, subString);
    printf("length of string: %d    length of substring: %d     threads: %d\n",
     stringLength, subStringLength, threadQuantity);

    //creating n threads
    newThread = (pthread_t*) malloc (sizeof(pthread_t) * threadQuantity);
    for (int i = 0; i < threadQuantity; ++i)
        pthread_create(&newThread[i], NULL, Substrings, NULL);
    

    //wait for all threads to finish.
    for (int i = 0; i < threadQuantity; ++i)
        pthread_join(newThread[i], NULL);
        

    printf("%lld\n", counter);
    free(newThread);
    return 0;
}


void* Substrings(void* arg)
{
    int position = 0;
    //local variable used to check if it was a substring
    unsigned char isEqual = FALSE;
    while(currentPosition < stringLength)
    {
        position = currentPosition;
        //if the first position is true, then assign true to isEqual and begin substring check
        if(string[position] == subString[0])
        {
            isEqual = TRUE;
            //checks to see if all positions are equal
            for (unsigned char i = 1; i < subStringLength-1; i++)
            {   //if a position isn't equal, then put isEqual to false and get out of loop
                if(string[position + i] != subString[i])
                {
                    isEqual = FALSE;
                    break;
                }
                
            }
            //if isEqual is true after all checks, increment counter
            if(isEqual == TRUE)
            {
                pthread_mutex_lock(&mutex);
                counter ++;
                pthread_mutex_unlock(&mutex);
            }
            
        }
        pthread_mutex_lock(&mutex);
        currentPosition++;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
    return NULL;
}

int GetThreadQuantity(char* string, char* subString)
{
    stringLength = strlen(string);
    subStringLength= strlen(subString);
    unsigned short threadQuantity = (stringLength/subStringLength) - 1;
    return threadQuantity;
}