#include <sys/types.h>
#include <string.h>

ssize_t find_short(const char *s) //ssize_t return originally on codewars
{
  int shortestWord = -1;
  int wordCount = 0;
  
  while (*s != '\0')
  {    
    if (*s == ' ')
    {
      if (shortestWord == -1)
      {
        shortestWord = wordCount;
      }
      
      if (wordCount < shortestWord)
      {
        shortestWord = wordCount;
      }
      
      wordCount = 0;
    }
    else
    {
      wordCount++;
    }
    s++;
  }
  
  if (wordCount < shortestWord)
  {
    shortestWord = wordCount;
  }
  
  if (shortestWord == -1){
    return wordCount;
  }
  printf("%d\n", shortestWord);
  return shortestWord;
}

/* Alternative solution
ssize_t find_short(const char *s)
{
    ssize_t shortest;
    ssize_t current;
  
    shortest = LONG_MAX;
    current = 0;
    for (size_t i = 0; s[i]; i++){
        if (s[i] == ' '){
            if (current < shortest)
                shortest = current;
            current = 0;
        }
        else
            current++;
    }
    if (current < shortest)
        return current;
    return shortest;
}
*/