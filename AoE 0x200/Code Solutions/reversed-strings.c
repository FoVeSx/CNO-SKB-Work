char *strrev (char *string)
{
  int stringLength = 0;
  char temp = 'a';
  int fp = 0;
  
  if (string[0] == '\0'){
    return string;
  }
  
  // grab last index of string
  for(int i = 0; string[i]; i++){
    stringLength++;
  }
  
  stringLength -= 1;
  
  if (stringLength == 0){
    return string;
  }

  while(1)
  {
    temp = string[fp];
    string[fp] = string[stringLength];
    string[stringLength] = temp;

    if (fp == stringLength) // odd case
    {
      break;
    }
    
    if ((stringLength - fp) == 1) // even case
    {
      break;
    }
    
    fp++;
    stringLength--;   
  }
  
  return string; // reverse the string in place and return it
}

/*
badass solution
char *strrev (char *string)
{
    char *l = string, *r = &string[strlen(string) - 1], temp;
    while (l <= r)
    {
        temp = *l;
        *l++ = *r;
        *r-- = temp;
    }
    return string;
}
*/