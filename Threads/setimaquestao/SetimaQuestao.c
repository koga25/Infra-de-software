#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define MAXBUFFERSIZE 20000000

enum RGB
{
    RED = 0,
    GREEN = 1,
    BLUE = 2
};

//apenas para poder guardar quando cada thread começa e qual a posição do array temporário rgbValues o thread vai colocar.
typedef struct s_helper
{
    size_t offsetOfSpecificThread;
    size_t positionOfArray;
}helper;

u_int8_t* rgbValues;
size_t rgbQuantityPerthread;
size_t width = 0;
size_t heigth = 0;
size_t bufferLength;
size_t beginningOfColors;
size_t rgbQuantity = 0; //todos os números R + G + B da imagem.
unsigned char buffer[MAXBUFFERSIZE];
unsigned char outFile[MAXBUFFERSIZE];

void* max(void* a, void* b);
void* ChangeColors(void* arg);

int main()
{
    int c;
    pthread_t* newThread;

    FILE* ppmFile = fopen("/home/lucas/Downloads/Threads/setimaquestao/test_muleflowers.ppm", "rb");

    if(ppmFile == NULL)
    {
        printf("Couldn't open file\n");
        exit(1);
    }

    fseek(ppmFile, 0L, SEEK_END);
    bufferLength = ftell(ppmFile);
    rewind(ppmFile);

    //ler até chegar no EOF
    for(size_t i = 0; (c = getc(ppmFile)) != EOF; i++)
    {
        buffer[i] = c;
    }
    //não esquecer de colocar o EOF no array
    buffer[bufferLength] = c;

    rewind(ppmFile);


    //ler até o segundo \n, a próxima linha é a largura e a altura da imagem 
    for(u_int8_t i = 0; i < 2; i++)
    {
        while((c = getc(ppmFile)) != '\n');
    }

    /*Pegando a largura da imagem, precisamos colocar esse primeiro while pois alguns arquivos estão com ' ' antes dos
    números de largura e altura*/
    while((c = getc(ppmFile)) == ' ')
    {

    }
    width *= 10; 
    width += c - 48; 
    while((c = getc(ppmFile)) != ' ')
    {
        if(c > 47 && c < 58)
        {
            width *= 10; 
            width += c - 48; 
        }
    }

    //Pegando a altura da imagem.
    while((c = getc(ppmFile)) != '\n')
    {
        if(c > 47 && c < 58)
        {
            heigth *= 10; 
            heigth += c - 48;
        }
    }

    //A próxima linha é onde começa os números RGB.
    while((c = getc(ppmFile)) != '\n');
    beginningOfColors = ftell(ppmFile);

    /*A quantidade de pixels que a tela tem é largura x altura. cada pixel tem 3 cores RGB, então só precisamos multiplicar
    o resultado por três para pegar o número total de cores RGB.*/
    rgbQuantity = width*heigth*3;

    fclose(ppmFile);

    //vendo qual entre a largura e a altura é maior.
    size_t widthOrHeigth =  (size_t)(max((void*) width, (void*) heigth));
    size_t threadQuantity = rgbQuantity/ widthOrHeigth;
    
    /*como o tamanho do número de cada RGB em ASCII é variável, (por exemplo, 255 são 3 caracteres, 76 são 2 caracteres)
    não tem como achar um espaçamento por bytes que vá garantir que pegaremos todos os caracteres de um número. A solução
    é achar a quantidade de números rgb totais e dividir o trabalho para as threads*Como também não sabemos qual a
    posição do array buffer que cada thread vai começar, é preciso implementar um contador que chegue até a o offset
    de números RGB que precisamos. Implementamos isso com um contador que vai contar quantos caracteres ' ' ou '\n' até
    chegar na variavel offsetOfSpecificThread, pois cada número é separado por um ou mais caracteres ' ' e/ou um caracter
    '\n'*/
    rgbQuantityPerthread = rgbQuantity/threadQuantity;

    //array temporário que vai guardar todos os valores rgb modificados em ordem.
    rgbValues = (u_int8_t*) calloc (rgbQuantity, sizeof(u_int8_t));
    
    newThread = (pthread_t*) malloc (sizeof(pthread_t) * threadQuantity);
    for(u_int64_t i = 0; i < threadQuantity; i++)
    {
        helper* helperVariable = (helper*) malloc (sizeof(helper));
        helperVariable->offsetOfSpecificThread = rgbQuantityPerthread * i; 
        helperVariable->positionOfArray = i;
        pthread_create(&newThread[i], NULL, ChangeColors, (void*) helperVariable);
    }

    for(u_int64_t i = 0; i < threadQuantity; i++)
        pthread_join(newThread[i], NULL);

    /*começo da transferência do array temporário de rgbs para o array buffer que irá ser utilizado para o output.
    bufferPosition indica a posição atual dos valores RGB, pixels vai ser utilizado para colocar os caracteres ' '
    e '\n' nas novas posições e o photoSize é o novo tamanho do arquivo, pois ao mudar os valores RGB a quantidade de
    caracter por valor RGB pode ser modificado, modificando assim o tamanho do arquivo.*/
    size_t bufferPosition = beginningOfColors;
    int pixels = 0;
    size_t photoSize = beginningOfColors + 1;
    for(size_t i = 0; i < rgbQuantity; i++)
    {
        //pegando o valor rgb e colocando os caracteres nas posições corretas.
        u_int8_t number;
        number = rgbValues[i++] + rgbValues[i++] + rgbValues[i];
        for(size_t j = 0; j < 3; j++)
        {
            u_int8_t firstNonZero = FALSE;
            for(size_t k = 0; k < 3; k++)
            {
                u_int8_t tempInt = number/(pow(10, 3 - 1-k));
                tempInt = tempInt%10;
                if(tempInt == 0 && firstNonZero == FALSE)
                {
                    //buffer[bufferPosition] = 48;
                }
                else
                {
                    buffer[bufferPosition] = tempInt + 48;
                    number/10;
                    bufferPosition++;
                    firstNonZero = TRUE;
                    photoSize++;
                    //firstNonZero = TRUE;
                }   
            }
            if(firstNonZero == FALSE)
            {
                buffer[bufferPosition] = 48;
                bufferPosition++;
                photoSize++;
            }
            if(pixels == 11)
            {
                buffer[bufferPosition] = '\n';
                pixels = 0;
                photoSize++;
            }
            else
            {
                buffer[bufferPosition] = ' ';
                pixels++;
                photoSize++;
            }
            bufferPosition++;
            firstNonZero = FALSE;
        }
    }

    FILE* out = fopen("out.ppm", "w");
    if(out == NULL)
    {
        printf("Failed to open file\n");
        exit(1);
    }
    if(fwrite(buffer, photoSize, sizeof(unsigned char), out) < 0)
    {
        printf("Failed to write to file\n");
        exit(1);
    }

    fclose(out);
    free(newThread);
    return 0;
}

void* ChangeColors(void* arg)
{
    helper* helperVariable = (helper*) arg; 
    size_t counter;
    size_t startPosition = 0;

    //aqui contaremos até chegar a posição do número rgb que queremos para a thread.
    for(counter = 0; counter < helperVariable->offsetOfSpecificThread; counter++)
    {
        while(buffer[beginningOfColors + startPosition] != ' ' && buffer[beginningOfColors + startPosition] != '\n')
            startPosition++;
        
        startPosition++;

        //pode ter chance de que o próximo caracter também é ' ' ou '\n', portanto vá até o próximo caracter de número
        while(buffer[beginningOfColors + startPosition] < 48 || buffer[beginningOfColors + startPosition] > 57)
            startPosition++;

        
    }


    u_int8_t rgb = (counter%3);
    float multiplyValue;
    switch(rgb)
    {
        case RED:
            multiplyValue = 0.3;
            break;
        case GREEN:
            multiplyValue = 0.59;
            break;
        case BLUE:
            multiplyValue = 0.11;
            break;  
    }

    size_t valuesAssigned = 0;
    for(size_t i = 0; valuesAssigned < rgbQuantityPerthread; i++)
    {
        unsigned char tempChar = buffer[beginningOfColors + startPosition + i];
        if(tempChar > 47 && tempChar < 58)
        {

            u_int8_t number = buffer[beginningOfColors + startPosition + i] - 48;

            for(size_t j = 1; j < 3; j++)
            {
                i++;
                u_int8_t tempChar2 = buffer[beginningOfColors + startPosition + i];
                if(tempChar2 == ' ' || tempChar2 == '\n')
                    break;
                
                number *= 10;
                number += tempChar2 - 48;
            }
            //printf("number[%i] to [%f] in position [%ld] of array, rgb of [%i]\n", number, number * multiplyValue, counter + valuesAssigned, rgb);
            number *= multiplyValue;
            rgbValues[counter + valuesAssigned] = number;
            valuesAssigned++;

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
    pthread_exit(NULL);
}

void* max(void* a, void* b)
{
    return a > b ? a : b;
}