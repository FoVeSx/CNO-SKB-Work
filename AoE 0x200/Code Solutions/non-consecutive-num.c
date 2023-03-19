// For c the function should return false if no non consecutive numbers are found
// It should return true if one is found and the pointer arg first set to the value

#include <stdbool.h>
#include <stdlib.h>

bool checkDistance(int x, int y)
{
  if (abs(x - y) != 1)
    return false;
  
  return true;
}

bool firstNonConsecutive (const int arr[], const int length, int *first)
{
  int tempPrev = arr[0];
  
  for (int i = 1; i < length; i++)
  {
    if (tempPrev > arr[i] && arr[i] < 0) // if previous is greater and -, break
    {
      *first = arr[i];
      return true;
    }
    
    if (tempPrev > arr[i]) // if previous is greater, break
    {
      *first = arr[i];
      return true;
    }
    
    if (!checkDistance(tempPrev, arr[i]))
    {
      *first = arr[i];
      return true;
    }
    
    tempPrev = arr[i];
  }
   
  
  return false;
}