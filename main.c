#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct node {
    char *filename;
    char *filepath;
};

int fileCounter;
struct node *newNode;
struct dirent *dirEntry;
pthread_mutex_t fileLock;

void *copyFiles() {
    int readChar;
    char backupFile[256];
    char currrentFile[256];
    FILE *source, *destination;

    // printf("Source filename: %s\n", newNode->filename);
    // printf("Source file: %s\n", newNode->filepath);

    strcpy(backupFile, "");
    strcat(backupFile, ".backup/");
    strcat(backupFile, dirEntry->d_name);

    strcpy(currrentFile, "");
    strcat(currrentFile, newNode->filepath);
    strcat(currrentFile, "/");
    strcat(currrentFile, newNode->filename);

    // printf("backupFile: %s\n", backupFile);
    // printf("currentFile: %s\n", currrentFile);

    source = fopen(currrentFile, "r");
    if(source == NULL) {
        fprintf(stderr, "Failed to open source file: %s\n", strerror(errno));
        exit(1);
    }

    destination = fopen(backupFile, "w");
    if(destination == NULL) {
        fprintf(stderr, "Failed to create backup file: %s\n", strerror(errno));
        exit(1);
    }

    if(pthread_mutex_lock(&fileLock) != 0) {
        fprintf(stderr, "Failed to lock mutex: %s\n", strerror(errno));
        exit(1);
    }

    while((readChar = fgetc(source)) != EOF) {
        // printf("readChar: %c\n", readChar);
        fputc(readChar, destination);
    }
    printf("File copied.\n");
    fclose(source);
    fclose(destination);

    if(pthread_mutex_unlock(&fileLock) != 0) {
        fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(errno));
        exit(1);
    }
    pthread_exit(NULL);
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
int countFiles(char *cwd) {
    DIR *dir;
    char newPath[256];

    dir = opendir(cwd);
    // printf("cwd: %s\n", cwd);
    if(dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", strerror(errno));
        return 1;
    }

    while((dirEntry = readdir(dir)) != NULL) {
        if(dirEntry->d_type == DT_REG) {
            printf("DT_REG: %s\n", dirEntry->d_name);
            sleep(1);
            newNode->filename = dirEntry->d_name;
            newNode->filepath = cwd;
            // printf("newNode->filename: %s\n", newNode->filename);
            // printf("newNode->filepath: %s\n", newNode->filepath);
            threadHandler();
            fileCounter++;
        }
        else if(dirEntry->d_type == DT_DIR) {
            if(strcmp(dirEntry->d_name, ".") == 0) {
                continue;
            }
            else if(strcmp(dirEntry->d_name, "..") == 0) {
                continue;
            }
            else if(strcmp(dirEntry->d_name, ".backup") == 0) {
                continue;
            }
            else {
                printf("DT_DIR: %s\n", dirEntry->d_name);
                sleep(1);
                strcpy(newPath, "");
                strcat(newPath, cwd);
                strcat(newPath, "/");
                strcat(newPath, dirEntry->d_name);
                countFiles(newPath);
            }
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
    char cwd[256];
    fileCounter = 0;
    dirEntry = NULL;
    newNode = malloc(sizeof(struct node *));
    //Check for -r command
    if(argc > 1) {
        //Call function to restore all backup files from .backup/
    }

    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Failed to get current working directory: %s\n", strerror(errno));
        return 1;
    }

    if(directoryHandler() == 1) {
        exit(1);
    }
    else {
        countFiles(cwd);
    }

    return 0;
}