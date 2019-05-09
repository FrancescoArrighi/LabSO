#include "myheader.h"
//ritorna la dimensione della stringa
long long int int_string(long long int a, char ** str){
  int segno = 1;
  if(a < 0){ // a = |a|
    segno = 0;
    a *= -1;
  }

  long long int i, temp = a;
  for(i = 1; temp > 127;  i++){
    temp = temp/127;
  }
  //printf("%d\n", i);
  *str = (char *) malloc(sizeof(char) * i+2);

  temp =  a;
  (*str)[0] = segno+1; // 1 se a < 0; 2 se a >= 0
  i = 1;
  while (temp > 1) {
    (*str)[i] = (temp % 127) + 1;
    temp =  temp/127;
    //printf("%d\n", (*str)[i]);
    i++;
  }
  (*str)[i] = '\0';
  return (i+1);
}

long long int string_int(char ** str){
  int segno = (*str)[0]-1;

  long long int i;
  long long int rt = 0;
  for(i = 0; (*str)[i] != '\0'; i++);
  i--;
  while (i > 0) {
    rt *= 127;
    rt += (*str)[i]-1;
    //printf("%d\n", (*str)[i]);
    i--;
  }
  if(segno == 0){
    rt *= -1;
  }
  return rt;
}

int main(int argc, char const *argv[]) {
  char * str;
  long long int a = 1557390525155155739;
  long long int b = -101;
  a = int_string(a, &str);
  printf("\n----------------\n%lld\n-----------------\n", a);
  b = string_int( &str);
  printf("%lld\n", b);
  return 0;
}
