#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define MAXBUFFERSIZE 2000000

enum RGB
{
    RED = 0,
    GREEN = 1,
    BLUE = 2
};

typedef struct s_helper
{
    size_t offsetOfSpecificThread;
    u_int8_t positionOfArray;
}helper;

u_int8_t** rgbValues;
size_t rgbQuantityPerthread;
size_t offset = 0;
size_t width = 0;
size_t heigth = 0;
size_t bufferLength;
size_t beginningOfColors;
size_t rgbQuantity = 0; // every R+G+B numbers that exist in the ppm file
unsigned char string[200];
int i = 0;
unsigned char buffer[MAXBUFFERSIZE];
unsigned char outFile[MAXBUFFERSIZE];

void* max(void* a, void* b);
void* ChangeColors(void* arg);

int main()
{
    int c;
    pthread_t* newThread;

    FILE* ppmFile = fopen("/home/lucas/Downloads/Threads/setimaquestao/Tiny6pixel.ppm", "rb");

    if(ppmFile == NULL)
    {
        printf("Couldn't open file\n");
        exit(1);
    }

    fseek(ppmFile, 0L, SEEK_END);
    bufferLength = ftell(ppmFile);
    rewind(ppmFile);

    for(size_t i = 0; (c = getc(ppmFile)) != EOF; i++)
    {
        buffer[i] = c;
    }
    //put EOF into 
    buffer[bufferLength] = c;

    rewind(ppmFile);

    /*while((c = getc(ppmFile)) != EOF)
    {
        string[i++] = c;
    }*/

    //read until the second \n char, then next line is the width and height of image
    for(u_int8_t i = 0; i < 2; i++)
    {
        while((c = getc(ppmFile)) != EOF && c != '\n');
    }

    //get width of image
    while((c = getc(ppmFile)) != EOF && c != ' ')
    {
        //add one decimal digit to width
        width *= 10; 
        //then add the number to the decimal digit
        width += c - 48;
    }

    //get heigth of image
    while((c = getc(ppmFile)) != EOF && c != '\n')
    {
        //add one decimal digit to heigth
        heigth *= 10; 
        //then add the number to the decimal digit
        heigth += c - 48;
    }

    //get the position of the file that begins to code the colors
    while((c = getc(ppmFile)) != EOF && c != '\n');
    //getc(ppmFile);
    beginningOfColors = ftell(ppmFile);

    //get how many R + G + B numbers exist;
    /*while((c = getc(ppmFile)) != EOF)
    {
        if(c == ' ' || c == '\n')
            rgbQuantity++;
    }*/
    rgbQuantity = width*heigth*3;

    fclose(ppmFile);

    size_t widthOrHeigth =  (size_t)(max((void*) width, (void*) heigth));
    size_t threadQuantity = rgbQuantity/ widthOrHeigth;
    //creating a pointer to pointer of u_int8_t to save the rgb values to create the
    //file later, need to do this because the size of the file can change.
    rgbValues = (u_int8_t**) malloc (sizeof(u_int8_t*) * threadQuantity);
    rgbQuantityPerthread = rgbQuantity/threadQuantity;
    for(size_t i = 0; i < threadQuantity; i++)
    {
        rgbValues[i] = (u_int8_t*) calloc (rgbQuantityPerthread, sizeof(u_int8_t));
    }

    //rounding up
    offset = ((bufferLength - beginningOfColors) + (threadQuantity - 1))/threadQuantity;
    
    newThread = (pthread_t*) malloc (sizeof(pthread_t) * threadQuantity);
    for(u_int64_t i = 0; i < 1; i++)
    {
        helper* helperVariable = (helper*) malloc (sizeof(helper));
        helperVariable->offsetOfSpecificThread = rgbQuantityPerthread * i; 
        helperVariable->positionOfArray = i;
        pthread_create(&newThread[i], NULL, ChangeColors, (void*) helperVariable);
    }

    for(u_int64_t i = 0; i < 1; i++)
        pthread_join(newThread[i], NULL);

    free(newThread);

    int bufferPosition = beginningOfColors;
    int pixels = 0;
    for(size_t i = 0; i < threadQuantity; i++)
    {
        for(size_t j = 0; j < rgbQuantityPerthread; j++)
        {
            
            u_int8_t number;
            number = rgbValues[i][(j++)] + rgbValues[i][(j++)] + rgbValues[i][(j)];
            
            for(size_t q = 0; q < 3; q++)
            {
                u_int8_t firstNonZero = FALSE;
                for(size_t k = 0; k < 3; k++)
                {
                    u_int8_t tempInt = number/(pow(10, 3 - 1-k));
                    tempInt = tempInt%10;
                    if(tempInt == 0 && firstNonZero == FALSE)
                    {
                        buffer[bufferPosition] = 48;
                    }
                    else
                    {
                        buffer[bufferPosition] = tempInt + 48;
                        number/10;
                        bufferPosition++;
                        //firstNonZero = TRUE;
                    }   
                }

                if(pixels == 11)
                {
                    buffer[bufferPosition] = '\n';
                    pixels = 0;
                }
                else
                {
                    buffer[bufferPosition] = ' ';
                    pixels++;
                }
                bufferPosition++;
            }
        }
    }

    FILE* out = fopen("out.ppm", "w");
    if(out == NULL)
    {
        printf("Failed to open file\n");
        exit(0);
    }
    if(fwrite(buffer, bufferLength, sizeof(unsigned char), out) < 0)
    {
        printf("Failed to write to file\n");
        exit(0);
    }

    fclose(out);
    return 0;
}

void* ChangeColors(void* arg)
{
    helper* helperVariable = (helper*) arg; 

    //counting ultil we arrive to the position of the pixels for this thread
    size_t counter;
    size_t startPosition = beginningOfColors;
    for(counter = 0; counter < helperVariable->offsetOfSpecificThread; counter++)
    {
        while(buffer[startPosition] != ' ' || buffer[startPosition] != '\n')
            startPosition++;
        //incrementing 1 to go to the next number
        startPosition++;
    }

    u_int8_t rgb = 0;
    float multiplyValue = 0.3;
    int rgbValuePosition = 0;
    u_int8_t valuesAssigned = 0;
    for(size_t i = 0; valuesAssigned < rgbQuantityPerthread || buffer[startPosition + i] == EOF ; i++)
    {
        unsigned char tempChar = buffer[startPosition + i];
        if(tempChar != ' ' && tempChar != '\n')
        {
            //we have three different chars to modify in the RGB 
            
            //we are reading from a ascii file, so if we have a value of 232 in the file
            //it uses 3 character spaces for every number ('2', '3', '2'), so we have to
            //parse to a number, modify the value, and then parse back.
            
            //adding first digit
            u_int8_t number = buffer[startPosition + i] - 48;

            u_int8_t numberLength = 1;
            for(size_t j = 1; j < 3; j++)
            {
                i++;
                //check if we have another digit.
                unsigned char tempChar2 = buffer[startPosition + i];
                if(tempChar2 == ' ' || tempChar2 == '\n')
                    break; //if don't have, exit loop
                
                //if we have, add digit to number
                number *= 10;
                number += tempChar2 - 48;
                numberLength++;
            }
            printf("adding number: (%d to %f)  to position: [%d][%d]  of array,  of:%d\n", number, number * multiplyValue, helperVariable->positionOfArray, rgbValuePosition, rgb);
            
            //multiplying value
            number *= multiplyValue;
            rgbValues[helperVariable->positionOfArray][rgbValuePosition] = number;
            rgbValuePosition++;
            valuesAssigned++;
            /*//parsing number into the string
            for(size_t j = 0; j < numberLength; j++)
            {
                u_int8_t tempInt = number/(pow(10, numberLength - 1-j));
                tempInt = tempInt%10;
                rgbValues[helperVariable->positionOfArray][i] = tempInt + 48;
                number/10;
                i++;
            }*/
            switch(rgb)
            {
                case RED:
                    multiplyValue = 0.59;
                    
                    rgb++;
                    break;
                case GREEN:
                    multiplyValue = 0.11;
                    rgb++;
                    break;
                case BLUE:
                    multiplyValue = 0.3;
                    rgb = 0;
                    break;  
            }
        }
        
    }
    free(arg);
    return NULL;
}

void* max(void* a, void* b)
{
    return a > b ? a : b;
}