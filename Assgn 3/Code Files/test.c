#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char* str = (char*)malloc(100*sizeof(char));
    strcpy(str, "H\ri\n");
    int correct = 0;
    printf("\\r");
}