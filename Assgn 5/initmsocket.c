#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void R(){}

void S(){}

int main(){
    pthread_t rid, sid;

    pthread_create(&rid, NULL, R, NULL);
    pthread_create(&sid, NULL, S, NULL);

    pthread_join(rid, NULL);
    pthread_join(sid, NULL);
}