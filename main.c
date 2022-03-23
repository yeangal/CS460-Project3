#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

struct node {
    char *filename;
    char *filepath;
};

int fileCounter;
int totalFiles;
int totalBytes;
struct node *newNode;
struct dirent *dirEntry;
pthread_mutex_t fileLock;
pthread_t thrID[32];

void restore() {

}

int compareFiles(char *backupFile, char *currentFile) {
    char oldFile[256];
    char newFile[256];
    struct stat attr1;
    struct stat attr2;

    strcpy(oldFile, backupFile);
    strcpy(newFile, currentFile);

    // printf("oldFile: %s\n", oldFile);
    // printf("newFile: %s\n", newFile);

    if(stat(oldFile, &attr1) != 0) {
        // fprintf(stderr, "Stat oldFile error: %s\n", strerror(errno));
        return 0;
    }

    if(stat(newFile, &attr2) != 0) {
        // fprintf(stderr, "Stat newFile error: %s\n", strerror(errno));
        return 0;
    }

    if(difftime(attr1.st_mtim.tv_sec, attr2.st_mtim.tv_sec) > 0) {
        // printf("%s Does not need backing up\n", newFile);
        return 1;
    }
    else if(difftime(attr1.st_mtim.tv_sec, attr2.st_mtim.tv_sec) < 0){
        // printf("%s is newer than %s\n", newFile, oldFile);
        return 2;
    }
}

void *copyFiles() {
    int byteCounter = 0;
    int readChar;
    char backupFile[256];
    char currrentFile[256];
    FILE *source, *destination;

    strcpy(backupFile, "");
    strcat(backupFile, ".backup/");
    strcat(backupFile, newNode->filename);
    strcat(backupFile, ".bak");

    strcpy(currrentFile, "");
    strcat(currrentFile, newNode->filepath);
    strcat(currrentFile, "/");
    strcat(currrentFile, newNode->filename);

    // printf("backupFile: %s\n", backupFile);
    // printf("currentFile: %s\n", currrentFile);

    if(compareFiles(backupFile, currrentFile) == 1) {
        printf("[Thread %d] %s Does not need backing up\n", fileCounter, newNode->filename);
        totalFiles--;
        pthread_exit(NULL);
    }
    else if(compareFiles(backupFile, currrentFile) == 2) {
        printf("[Thread %d] WARNING: Overwriting %s\n", fileCounter, backupFile);
    }
    else {
        printf("[Thread %d] Backing up %s\n", fileCounter, newNode->filename);
    }

    source = fopen(currrentFile, "r");
    if(source == NULL) {
        fprintf(stderr, "Failed to open source file: %s\n", strerror(errno));
        pthread_exit(NULL);
    }

    destination = fopen(backupFile, "w");
    if(destination == NULL) {
        fprintf(stderr, "Failed to create backup file: %s\n", strerror(errno));
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&fileLock);
    while((readChar = fgetc(source)) != EOF) {
        // printf("readChar: %c\n", readChar);
        fputc(readChar, destination);
        byteCounter++;
        totalBytes++;
    }
    printf("[Thread %d] Copied %d bytes from %s to %s\n", fileCounter, byteCounter, newNode->filename, backupFile);
    fclose(source);
    fclose(destination);

    pthread_mutex_unlock(&fileLock);
    pthread_exit(NULL);
}

void threadHandler() {
    pthread_mutex_init(&fileLock, NULL);
    if((pthread_create(&thrID[fileCounter], NULL, copyFiles, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
        exit(1);
    }

    if(pthread_join(thrID[fileCounter], NULL) != 0) {
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
void countFiles(char *cwd) {
    DIR *dir;
    char newPath[256];

    dir = opendir(cwd);
    if(dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", strerror(errno));
        return;
    }

    while((dirEntry = readdir(dir)) != NULL) {
        if(dirEntry->d_type == DT_REG) {
            sleep(1);
            newNode->filename = dirEntry->d_name;
            newNode->filepath = cwd;
            threadHandler();
            totalFiles++;
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
                sleep(1);
                strcpy(newPath, "");
                strcat(newPath, cwd);
                strcat(newPath, "/");
                strcat(newPath, dirEntry->d_name);
                countFiles(newPath);
            }
        }
    }
    // while(fileCounter > 0) {
    //     printf("fileCounter: %d\n", fileCounter);
    //     if(pthread_join(thrID[fileCounter], NULL) != 0) {
    //         fprintf(stderr, "Failed to join thread: %s\n", strerror(errno));
    //         exit(1);
    //     }
    //     fileCounter--;
    // }
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
        printf("Backup directory doesn't exist, creating dir .backup/\n");
        newDir = mkdir(".backup/", 0777);
        if(newDir == -1) {
            fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
            return 1;
        }
        else {
            printf("Directory .backup/ successfully created\n");
            return 0;
        }
    }
    else {
        printf("Backup directory already exists\n");
        closedir(dir);
        return 0;
    }
}

int main(int argc, char *argv[]) {
    char cwd[256];
    fileCounter = 0;
    totalFiles = 0;
    totalBytes = 0;
    dirEntry = NULL;
    newNode = malloc(sizeof(struct node *));
    //Check for -r command
    if(argc > 1) {
        //Call function to restore all backup files from .backup/
        restore();
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
    printf("Successfully copied %d files (%d bytes)\n", totalFiles, totalBytes);

    return 0;
}
