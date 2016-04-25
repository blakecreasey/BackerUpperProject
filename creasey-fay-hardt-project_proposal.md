Blake Creasey
Julia Fay
Madeleine Hardt

### Project Proposal: The "Backer Upper System" (working title)

#### Purpose and impact of the system:
* Take snapshots of a directory over time, monitoring changes
* Acts as a version control system and backup utility


#### Key areas covered:
* Persistence: monitors changes in a directory and saves these changes
  persistently 
* No second area covered extensively


#### Rough description of implementation:
##### Major components: 
* First backup: copy all files
* While loop waiting for (1)signal from inotify or (2)time interval to pass(TBD)
  * When a signal is received, we know that inotify has written a change to
    a special directory which tells us what has been changed in the
    directory that we are watching
    * We will have to decide whether to create a new backup whenever a signal
      is received or whenever a certain time interval has passed
  * Create a backup:
    * Make a new directory full of soft links to each file
    * If there is a new file, copy it into the new directory
    * If a file has been deleted, delete its soft link from the new directory
    * If a file has been renamed, rename its soft link in the new directory
    * If the special directory tells us there has been a change for a
      particular file, delete the soft link and make a copy of the altered
      file into the new directory
* Continue the while loop until we receive a quit command


##### Timeline:
* 4/21: Write project proposal
* 4/25: Read about inotify
* 4/28: Outline application; successfully integrate inotify so that we
       receive change notification about a directory
* 5/2: Be able to successfully create backups
* 5/5: Set backups to time schedule
* 5/8: Debug, prepare for presentation, and submit


##### Potential challenges in the implementation:
* Understanding inotify
* Timing policy design and implementation 
* Handling special cases, such as, rename, delete, and new file


#### Possible alternate solutions:
* Change backup scheduling policy (i.e. switch from time interval to signal
  or vice versa)
* Integrate with git
