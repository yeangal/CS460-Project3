#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

int fileCounter = 0;
pthread_mutex_t fileLock;


    //Lock Mutex
    //Stat() file
    //Unlock Mutex
    //Exit thread
void copyFiles(struct dirent *dirEntry) {
    struct stat buffer;
    while(pthread_mutex_lock(&fileLock) != 0) {}
    
}

void threadHandler(struct dirent *dirEntry) {
    int i = 0;
    pthread_t file[fileCounter];
    //Spawn new thread, call Copy() func

    pthread_mutex_init(&fileLock, NULL);
    if((pthread_create(&file[fileCounter], NULL, copyFiles, dirEntry)) != 0) {
        printf("Failed to create thread: file\n");
        exit(1);
    }
}

int countFiles(struct dirent *dirEntry) {
    DIR *dir = opendir("./");

    if(dir == NULL) {
        fprintf(stderr, "Failed to open directory ./: %s\n", strerror(errno));
        return 1;
    }

    if(dirEntry == NULL) {
        dirEntry = readdir(dir);
    }
    
    while(dirEntry != NULL) {
        if(dirEntry->d_type == DT_REG) {
            //Call threadHandler()
            threadHandler(dirEntry);
            fileCounter++;
        }
        else if(dirEntry->d_type == DT_DIR) {
            //Traverse directory and count files
            //Call countFiles(dirEntry)
            countFiles(dirEntry);
        }
    }
}

/*
Checking for directory .backup/
If directory exists, return pointer to directory stream
If directory doesn't exist, create it. If creation fails, set errno and return 1
If creation suceeds, return 0
*/
int directoryHandler() {
    int newDir;
    DIR *dir = opendir(".backup/");
    
    if(dir == NULL) {
        printf("Directory doesn't exist, creating dir .backup/\n");
        newDir = mkdir(".backup/", 0777);
        if(newDir == -1) {
            fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
            return 1;
        }
        else {
            printf("Directory .backup/ successfully created\n");
            return 0;;
        }
    }
    else {
        printf("Directory already exists\n");
        closedir(dir);
        return 0;
        // return dir;
    }
}

int main(int argc, char *argv[]) {
    
    //Check for -r command
    if(argc > 1) {
        //Call function to restore all backup files from .backup/
    }

    if(directoryHandler() == 1) {
        exit(1);
    }
    
    return 0;
}