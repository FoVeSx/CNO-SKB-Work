/*
To obtain the length of the output string without generating it, you can set n equal to zero; 
in this case, sprintf() writes nothing to dest, which may be a null pointer.
*/

#include <stdlib.h>
#include <stdio.h>

char *number_to_string(int number) {

  char *result = calloc(50, sizeof(char));
  sprintf(result, "%d", number);
  return result;

}

/*
#include <stdlib.h>

const char* number_to_string(int number) {

    size_t bufferLength = snprintf(NULL, 0, "%d", number); //Find the length of the string conversion
    char* buffer = malloc(bufferLength + 1); //dynamically allocate the string
    
    snprintf(buffer, bufferLength + 1, "%d", number); //populate the string
    
    return buffer; //return the string
}
*/