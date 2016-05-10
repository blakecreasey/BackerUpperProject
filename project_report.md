Blake Creasey
Julia Fay
Madeleine Hardt

### Project Report: The "Backer Upper System" (working title)

Your report must give an overview of your system, describe the design and implementation of the system, and finally present an evaluation of your work. Details for each section are below.

#####Overview 
Give a brief overview of your system’s design, the high level points of your implementation (e.g. ___ runs on the GPU, while the CPU handles ___ using a thread pool), and summarize your evaluation strategy and results.
Your overview should not exceed 400 words.

Preconditions: 
1. The user must specify the path of the backup directory which is hard-coded at the top of "main.c" and "backup_helpers.c".
2. The user must run the program with a command line argument that specifies the path of the directory to be watched, e.g. "./main ~/Desktop/CSC213". The user should not end the path for the watched directory with a slash, for example, write "." for current directory not "./"
3. Our system will not iterate through directories within a watched directory. Any file which the user wishes to backup must be directly in the watched directory.
4. The watched directory must be either "." or ".." (implementation challenge)

If it does not already exist, ~Desktop/backups is created at the onset of the program. 

Once the command to run the program with a specified watched directory has been executed, any changes made to files in that directory will be tracked. These events are received as signals from inotify as added as nodes (containing event type and file affected) to a queue. When the user terminates the program by pressing the "Enter" key, they are prompted to indicate whether they would like to back up the file changes made since the beginning of this program run or not (by entering "1" to back up or "0" to discard the directory change events). If the user presses "0" the program is ends.

If the user presses "1", a new time-stamped backup directory is created within the main backup folder and symbolic links to every file in the most recent backup are made. These links go directly to each file in the most recent backup directory rather than creating a copy of each file because until we iterate over the queue we will not know whether modifications have been made to each particular file and so whether a new copy is necessary. Then elements are taken off the front of the queue and appropriate actions taken until it is empty.

For each element in the queue, one of three things is done:
1. For a creation event, a copy of the created file is added to the new backup directory. 
2. For a deletion event, the symbolic link to the previous version of the deleted file is deleted from the new backup directory.
3. For a modification event, if the file of the same name in the new backup directory is not a symbolic link, we know we have already encounted a modification to this file and added a copy of the altered version to the new backup directory and therefore we have an updated version and do not need to make another copy. Otherwise, we delete the symbolic link and add a copy of the modified file to the new backup directory.


#####Design and Implementation



#####Evaluation 
