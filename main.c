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
struct node *newNode;
struct dirent *dirEntry;
pthread_mutex_t fileLock;

void restore() {

}

int compareFiles(char *backupFile, char *currentFile) {
    char oldFile[256];
    char newFile[256];
    // time_t oldTime;
    // time_t newTime;
    struct stat attr1;
    struct stat attr2;

    strcpy(oldFile, backupFile);
    strcpy(newFile, currentFile);

    // printf("oldFile: %s\n", oldFile);
    // printf("newFile: %s\n", newFile);

    if(stat(oldFile, &attr1) != 0) {
        fprintf(stderr, "Stat oldFile error: %s\n", strerror(errno));
        exit(1);
    }

    if(stat(newFile, &attr2) != 0) {
        fprintf(stderr, "Stat newFile error: %s\n", strerror(errno));
        exit(1);
    }

    if(difftime(attr1.st_mtim.tv_sec, attr2.st_mtim.tv_sec) > 0) {
        printf("Backup file up to date.\n");
        return 2;
    }
    else if(difftime(attr1.st_mtim.tv_sec, attr2.st_mtim.tv_sec) < 0){
        printf("New file is newer..\n");
        return 1;
    }
    else {
        printf("Both files are the same..\n");
        return 0;
    }
}

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

    if(compareFiles(backupFile, currrentFile) == 0) {
        pthread_exit(NULL);
    }
    else if(compareFiles(backupFile, currrentFile) == 2) {
        pthread_exit(NULL);
    }
    else {
        printf("Copying file to %s .backup\n", newNode->filename);
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

    if(pthread_mutex_lock(&fileLock) != 0) {
        fprintf(stderr, "Failed to lock mutex: %s\n", strerror(errno));
        pthread_exit(NULL);
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
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

void threadHandler(pthread_t *thrID) {

    pthread_mutex_init(&fileLock, NULL);
    if((pthread_create(thrID, NULL, copyFiles, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
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
    // printf("cwd: %s\n", cwd);
    if(dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", strerror(errno));
        return;
    }

    pthread_t thrIDs[32];
    int activeThreadCount = -1;

    while((dirEntry = readdir(dir)) != NULL) {
        if(dirEntry->d_type == DT_REG) {
            printf("DT_REG: %s\n", dirEntry->d_name);
            sleep(1);
            newNode->filename = dirEntry->d_name;
            newNode->filepath = cwd;
            // printf("newNode->filename: %s\n", newNode->filename);
            // printf("newNode->filepath: %s\n", newNode->filepath);
            printf("before threading\n");
            activeThreadCount++;
            threadHandler(&thrIDs[activeThreadCount]);
            printf("After threadHandler\n");
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
    printf("Before Join\n");
    while(activeThreadCount > -1) {
        if(pthread_join(thrIDs[activeThreadCount], NULL) != 0) {
            fprintf(stderr, "Failed to join thread: %s\n", strerror(errno));
            exit(1);
        }
        activeThreadCount--;
    }
    printf("After Join\n");

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
            return 0;
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
