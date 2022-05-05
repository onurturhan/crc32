#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAX_FILE_SIZE     0x1000000  //MAX 16MB
#define MAX_CRC_BLOCK     0x1000

#define SLOW_CRC_TIMEOUT  1000 /*usec*/

char inp[MAX_FILE_SIZE];
int srcSize;

void printProgress(char count)
{
    #define PB_STEP_SIZE       2
    #define PB_MAX_SIZE        100

    const char prefix[] = "\e[1ACRC CALCULATION: [";
    const char suffix[] = "]";
    const char prefix_length = sizeof(prefix) - 1;
    const char suffix_length = sizeof(suffix) - 1;
    char *buffer = calloc((PB_MAX_SIZE/PB_STEP_SIZE) + prefix_length + suffix_length + 1, 1); // +1 for \0
    char i = 0;

    strcpy(buffer, prefix);
    for (; i < (PB_MAX_SIZE/PB_STEP_SIZE); ++i)
    {
        buffer[prefix_length + i] = i < (count/PB_STEP_SIZE) ? '#' : ' ';
    }

    strcpy(&buffer[prefix_length + i], suffix);
    printf("\b%c[2K\r%s [%3d%%]\n", 27, buffer, count);
    fflush(stdout);
    free(buffer);
}

int readFile(const char* filename, char* src)
{
    FILE* in_file = fopen(filename, "rb");
    struct stat sb;

    if(!in_file)
    {
        perror("fopen");
        return(EXIT_FAILURE);
    }

    if (stat(filename, &sb) == -1)
    {
        perror("stat");
        return(EXIT_FAILURE);
    }

    srcSize = sb.st_size;

    fread(src, srcSize, 1, in_file);
#if 0
    printf("Read data : [0]0x%02X [1]0x%02X / LEN:%d \n", src[0], src[1], srcSize);
#endif
    fclose(in_file);

    return(EXIT_SUCCESS);
}

int main(int argc,  char* argv[])
{
    int Status;
    unsigned int crc;
      
    if(argc == 2)
    {
        if( (Status = readFile(argv[1], inp )) != EXIT_SUCCESS)
            goto destructor;
                
        crc = CalculateCrc(inp, srcSize);
        printf("Fast CRC32: \"%s\" => \"0x%08X\" \n", argv[1], crc);
    }
    else if(argc == 3)
    {
        if( (Status = readFile(argv[2], inp )) != EXIT_SUCCESS)
            goto destructor;
        
        unsigned int size   = srcSize;
        unsigned int offset = 0;
        
        CalculateCrc(NULL, 0) ;
        
        while(size > MAX_CRC_BLOCK)
        {        
            crc = CalculateCrc(&inp[offset], MAX_CRC_BLOCK);
            size   -= MAX_CRC_BLOCK;
            offset += MAX_CRC_BLOCK;
            
            usleep(SLOW_CRC_TIMEOUT); 
            printProgress((offset*100)/srcSize);
        }
           
        crc = CalculateCrc(&inp[offset], size);
        printProgress(100);
        
        printf("Slow CRC32: \"%s\" => \"0x%08X\" \n", argv[2], crc);    
    }
    else
    {
        printf("Usage:"
               "\n     CRC32 Fast Hash Calculator   =>  ./crc32 input_filepath"
               "\n     CRC32 Slow Hash Calculator   =>  ./crc32 -p input_filepath \n\n");               
               
                                 
        goto destructor;

    }

destructor:
    return 0;

}
