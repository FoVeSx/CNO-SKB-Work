#include <stddef.h>

//int sum_array(const int *values, size_t count);

int sum_array(const int *values, size_t count)
{
  // unnecessary check?
  if (count == 0){
    return 0;
  }
  
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    result += values[i];
  }
  
  return result;
  
}