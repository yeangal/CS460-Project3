#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char const *argv[]) {

    if(argc == 1) {
        printf("Backup Mode\n");
    }else if(argc == 2 && strcmp(argv[1], "-r") == 0) {
        printf("Restore Mode\n");
    }else {
        printf("Invalid input\n");
    }

    return 0;
}

void *fileThread() {
    return NULL;
}

void backupFile() {

}

void restoreFile() {

}
