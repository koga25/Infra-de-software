#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define MAXSTRINGSIZE 10000
#define MAXSUBSTRINGSIZE 100


u_int64_t counter = 0;
char string[MAXSTRINGSIZE];
char subString[MAXSUBSTRINGSIZE];
u_int64_t stringLength;
u_int64_t subStringLength;
u_int64_t  currentPosition = 0;
unsigned char offset = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* Substrings(void* arg);
void GetStringsLength(char* string, char* subString);
u_int64_t GetThreadQuantity(u_int64_t stringLength, u_int64_t substringLength);

u_int64_t main()
{
    u_int64_t i = 0;
    //initializing counter
    pthread_t* newThread;
    
    u_int64_t threadQuantity;

    //getting the strings
    FILE* file = fopen("file2.txt", "r");
    if(file == NULL)
    {
        printf("Couldn't open file");
        exit(0);
    }
    u_int64_t c;
    while((c = getc(file)) != EOF &&  c != ' ')
    {
        string[i++] = c;
    }
    i = 0;
    while((c = getc(file))&& c != EOF && c!= ' ' && c!= '\n')
    {
        subString[i++] = c;
    }
    fclose(file);

    /*printf("type your string (0 < n <= 800)\n");    
    fgets(string, sizeof(string), stdin);   
    printf("type your subString (0 < n <= 20)\n");
    fgets(subString, sizeof(subString), stdin);*/

    GetStringsLength(string, subString);
    threadQuantity = GetThreadQuantity(stringLength, subStringLength);

    printf("length of string: %d    length of substring: %d     threads: %d\n",
     stringLength, subStringLength, threadQuantity);

    //creating n threads
    newThread = (pthread_t*) malloc (sizeof(pthread_t) * threadQuantity);
    offset = stringLength/threadQuantity;

    
    for (u_int64_t i = 0; i < threadQuantity; ++i)
    {
        u_int64_t* offsetOfSpecificThread = (u_int64_t*)malloc(sizeof(u_int64_t));
        offsetOfSpecificThread[0] = offset * i;
        pthread_create(&newThread[i], NULL, Substrings, (void*) (offsetOfSpecificThread));
    }
    
    //wait for all threads to finish.
    for (u_int64_t i = 0; i < threadQuantity; ++i)
        pthread_join(newThread[i], NULL);
        

    printf("%lld\n", counter);
    free(newThread);
    return 0;
}


void* Substrings(void* arg)
{

    u_int64_t* currentPosition = (u_int64_t*)arg;
    u_int64_t maxPosition = currentPosition[0]+offset;
    //local variable used to check if it was a substring
    unsigned char isEqual = FALSE;
    while(currentPosition[0] < maxPosition)
    {
        //if the first position is true, then assign true to isEqual and begin substring check
        if(string[currentPosition[0]] == subString[0])
        {
            isEqual = TRUE;
            //checks to see if all positions are equal
            for (unsigned char i = 1; i < subStringLength; i++)
            {   //if a position isn't equal, then put isEqual to false and get out of loop
                if(string[currentPosition[0] + i] != subString[i])
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
        currentPosition[0]++;
    }
    free(arg);
    pthread_exit(NULL);
    return NULL;
}

void GetStringsLength(char* string, char* subString)
{
    stringLength = strlen(string);
    subStringLength= strlen(subString);
    unsigned short threadQuantity = (stringLength/subStringLength);
}

u_int64_t GetThreadQuantity(u_int64_t stringLength, u_int64_t substringLength)
{
    //stringLength mod threadQuantity = 0
    //substringLength < stringlength/threadquantity
    u_int64_t threadQuantity = floor((double) (stringLength/substringLength));
    
}
