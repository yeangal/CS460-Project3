#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int fileCounter;
pthread_mutex_t fileLock;
struct dirent *dirEntry;

void *copyFiles() {
    char backupFile;
    FILE *source, *destination;

    printf("dirEntry = %s\n", dirEntry->d_name);
    // strcat(backupFile, ".backup/");
    // source = fopen(dirEntry, "r");
    // if(source == NULL) {
    //     fprintf(stderr, "Failed to open source file: %s\n", strerror(errno));
    //     exit(1);
    // }
    // destination = fopen(".backup/", "w");
    // if(destination == NULL) {
    //     fprintf(stderr, "Failed to create backup file: %s\n", strerror(errno));
    //     exit(1);
    // }

    // if(pthread_mutex_lock(&fileLock) != 0) {
    //     fprintf(stderr, "Failed to lock mutex: %s\n", strerror(errno));
    //     exit(1);
    // }

    // while(1) {

    // }

    // if(pthread_mutex_unlock(&fileLock) != 0) {
    //     fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(errno));
    //     exit(1);
    // }
    // pthread_exit(NULL);
    
}

void threadHandler() {
    pthread_t file[fileCounter];

    pthread_mutex_init(&fileLock, NULL);
    if((pthread_create(&file[fileCounter], NULL, copyFiles, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
        exit(1);
    }
    
    if(pthread_join(file[fileCounter], NULL) != 0) {
        fprintf(stderr, "Failed to join thread: %s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_destroy(&fileLock);
}

/*
Opens current directory and iterates through each file
If file is a regular file, call threadHandler() and increment fileCounter
If file is a sub directory, recursively call countFiles()
*/
int countFiles() {
    char cwd[256];
    DIR *dir;

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Failed to get current working directory: %s\n", strerror(errno));
        return 1;
    }

    dir = opendir(cwd);
    printf("cwd: %s\n", cwd);
    if(dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", strerror(errno));
        return 1;
    }

    printf("Check 1\n");
    sleep(2);
    if(dirEntry == NULL) {
        dirEntry = readdir(dir);
        if(dirEntry == NULL) {
            fprintf(stderr, "Failed to read directory: %s\n", strerror(errno));
            return 1;
        }
    }

    printf("Check 2\n");
    sleep(2);
    while((dirEntry = readdir(dir)) != NULL) {
        printf("Check 3\n");
        sleep(2);
        if(dirEntry->d_type == DT_REG) {
            printf("Check 4: %s\n", dirEntry->d_name);
            sleep(2);
            threadHandler();
            fileCounter++;
        }
        else if(dirEntry->d_type == DT_DIR) {
            printf("Check 5: %s\n", dirEntry->d_name);
            sleep(2);
            countFiles();
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
    }
}

int main(int argc, char *argv[]) {
    fileCounter = 0;
    dirEntry = NULL;

    //Check for -r command
    if(argc > 1) {
        //Call function to restore all backup files from .backup/
    }

    if(directoryHandler() == 1) {
        exit(1);
    }
    else {
        countFiles();
    }
    
    return 0;
}