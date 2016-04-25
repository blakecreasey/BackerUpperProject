Blake Creasey
Julia Fay
Madeleine Hardt

### Project Proposal: The "Backer Upper System" (working title)

#### Purpose and impact of the system:
The purpose of our system is to take snapshots of a directory over time, monitoring changes. This acts as a version control system and backup utility. We would like to user to be able to track changes made to files in a directory by looking back at a separate file for each version. We plan to generate backups based on time intervals so that the user does not have to save to track the state of the file at that time. We plan to begin deleting backups after a certain time period has passed. For example, after a day we would keep only one backup older than a day and after a week we would keep only one backup older than a week. This allows us to give the user both short-term and long-term persistence without sacrificing too much memory. We allow the user persistently track changes as they are being made by frequently backing up the directory, but also maintaining (far fewer) older backups by routinely discarding backups from a similar time period.


#### Key areas covered:
Our project will cover persistence. It will monitors changes in a directory and saves these changes persistently. The project will not cover a second area extensively.


#### Rough description of implementation:
##### Major components: 
Our first step will be to perform the first backup by copying all the files to a new directory. We will have a while loop that will either wait for the first signal from inotify or for a pre-determined time interval to pass. We will have to make the decision to either use inotify signals or a timer. Receiving a signal indicates that inotify has written a change to a special directory. The directory contents will tell us what has been changed in the directory that we are watching. To create a backup, we will make a new directory full of soft links to each file. If there is a new file we will copy it into the new directory. If a file has been deleted, we will delete its soft link from the new directory. If a file has been renamed, we will rename its soft link in the new directory. If a special directory tells us there has been a change for a particular file, delete the soft link and make a copy of the altered file into the new directory. We will continue with this while loop until we receive a quit command.


##### Timeline:
* 4/21: Write project proposal
* 4/25: Read about inotify
* 4/28: Outline application; successfully integrate inotify so that we
       receive change notification about a directory
* 5/2: Be able to successfully create backups
* 5/5: Set backups to time schedule
* 5/8: Debug, prepare for presentation, and submit


##### Potential challenges in the implementation:
We anticipate that it may be challenging to understand inotify, choose an implement a backup-interval policy, and handle special cases, such as, renamed files, added files, and deleted files. We plan to use online resources to learn how to use inotify (e.g. http://www.thegeekstuff.com/2010/04/inotify-c-program-example/). We plan to use a time-interval backup policy using a timer and we think we will be able to handle special cases by addressing them first in each backup, that is, adding, deleting, and renaming before we check for signaled changes to files. 


#### Possible alternate solutions:
If we need to use an alternate solution, we could try changing out backup-interval policy from time-based backups to signal-based backups (i.e. backup whenever a signal is received instead of after a certain time interval has passed). We could also try integrating our system with git to use some of the infrastructure that is already in place. 
