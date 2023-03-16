#include <stdbool.h>

const char *bool_to_word (bool value)
{
// you can return a static/global string or a string literal
  char *resultTrue = "Yes";
  char *resultFalse = "No";
  
  if (value == false){
    return resultFalse;
  }
  return resultTrue;
  
}

/*
Solutions that I reviewed after submission

const char *bool_to_word (bool value)
{
        return value ? "Yes" : "No";
}

const char *bool_to_word (int value){
  if(value)
    return "Yes";
  else
    return "No";
}

*/