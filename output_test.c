#ifdef __clang__
    #pragma clang diagnostic error "-Wshadow-all"
    #pragma clang diagnostic error "-Wreturn-type"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define OFD_OS_WINDOWS
#endif

#ifdef OFD_OS_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif


#define OFD_IMPLEMENTATION
#include "ofd.h"

#include <stdio.h>

int main(int num_arguments, char** arguments)
{
    if(num_arguments != 3)
    {
        printf("Expected 2 arguments, got %d.", num_arguments? num_arguments - 1 : 0);
        return -1;
    }
    
    Ofd_String file1, file2;
    if(!ofd_read_whole_file(arguments[1], &file1)) return -1;
    if(!ofd_read_whole_file(arguments[2], &file2)) return -1;
    
    if(!ofd_strings_match(file1, file2))
    {
        printf("%s and %s do not match!", arguments[1], arguments[2]);
        return -1;
    }
    
    return 0;
}