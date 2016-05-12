Blake Creasey
Julia Fay
Madeleine Hardt

### Project Report: The "Backer Upper System" (working title)

Your report must give an overview of your system, describe the design and implementation of the system, and finally present an evaluation of your work. Details for each section are below.

#####Overview (This is over 400 words right now and is probably too detailed, I am going to put some of it into the the design and implementation, we need to figure out what should go where, otherwise we will be super repeditive)
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

The design and implementation section should explain the structure of your system and, importantly, the rationale for that structure. Design decisions include details like the data structures and algorithms you used, the places where you used concurrency or decided to run code on the GPU, and any libraries you may have used for your implementation.

Try to organize this section in a top down manner: introduce the major components of your system, outline the responsibilities of each component, and then jump into the specifics of each component. You may want to repeat this process for sub-components if there are reasonable logical divisons within the major pieces of your project.

Another strategy for organizing this section would be to break it down by concerns. System concerns include updating a file index in response to a file deletion, maintaining game state over a network connection, or other general requirements that may not map to specific units of code in your system. If you can identify a handful of high-level concerns it may make more sense to discuss the overall implementation structure and then describe how each concern fits into this structure.

In describing your system, identify at least two principles from Butler Lampson’s Hints for Computer System Design that you used in building your system. In addition to identifying these hints, reflect on how they might (or might not) have helped you build a working implementation.

Your design section should be approximately two pages of text, plus figures where appropriate.

#####Evaluation

You are required to present an empirical evaluation of your system. The details of this evaluation will depend on your project topic, but there are a few common requirements. First, you must describe the experimental set-up; this includes the hardware and software you are using for the evaluation, versions of any important software tools (including libraries), and the methods you use for gathering data (e.g. we measure execution time using the time command). Then, you should measure an appropriate aspect of your system’s behavior while varying some aspect of the load on your system or its environment. Make sure your evaluation section explains what you are trying to measure and discusses an interpretation of your results.

For a file indexing project, you may want to measure the time it takes to issue a query as a function of the total size of the files in your index. For a project that uses a GPU, you might want to explore how varying block size changes the performance of your system. Do your best to pick an evaluation dimension that fits with your project goals; if the objective is to make a particular computation faster, then measure the speed of that computation. For some projects the evaluation will be less natural. I am happy to discuss this with you.

Your evaluation section should include at least one graph with a minimum of 8 data points relating to your system (16 if you compare to another system).

(Insert graph into report)

To evaulate our system we decided to time our system, using the library <time.h>. We varied the number of files, by orders of 2. We kept our modifications constant, modifying one file each time and then calling for a backup. The timer recorded the time it took for our system to complete one full backup. Because there was only one modification, the time would be mostly dependent on creation of softlinks backwards and the rest of system costs. We plotted our data points to a graph and noticed a linear trend. Time increased linearly as we incremented number of files to be backed up. This is a positive trend. Theoretically we would want our system to maintain a constant speed with increased workload, which is what we appeared to have accomplished. To further our evaluation we would want to time our system under a varying number of creations, deletions and modifications.  

