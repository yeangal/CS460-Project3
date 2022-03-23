# CS460-Project3
File Backup System (BackItUp!)

- Program Details
  - [1] Program creates a backup hidden backup directory called ".backup/"
  - [2] Program iterates through the current working directory and subdirectories incrementing a counter for each regular file encountered
  - [3] Program calls on ThreadHandler() to spawn a thread for each regular file counted
  - [4] Program compares files from the current working directory to its backup. If the backup file is older then it's overwritten, else, do nothing
  - [5] Program copies all other files to .backup/
  - [6] Optional -r command allows for the restoration of all newer files contained in the backup directory


- How to run:
  - [1] Compile by calling 'make' in the directory containing the Makefile and main.c
    - ./main [-r]
