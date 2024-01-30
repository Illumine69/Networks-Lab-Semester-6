#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char* ch = (char*)malloc(10*sizeof(char));
    strcpy(ch, "Hello\n\0Hi");
    printf("%d", (int)strlen(ch));
}