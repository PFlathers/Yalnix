### Checkpoint 3 - Note, that we are taking an extention (2 cupons)


##### 0. If your code is not in the main branch, how do we check it out?

- code is contained in _feat.checkpoint3_ branch. To access it, use [this link](https://gitlab.cs.dartmouth.edu/bjuncek/cs58-F17-droptables/repository/feat.checkpoint3/archive.zip) for a direct download or `git clone -b feat.checkpoint3 <prefered_repo_link>`. Note that we do not provide repo link as we don't know your prefered mode of authentification.


##### 1. Did you make the checkpoint? 
Kindof. 
We got the big part (kernel switching and init process) to work, however, what seems to be bothering us is the delay. Namely, our delay works compiles and theoretically should run, but doesn't really block for 3 cycles as we don't have the way to handle blocked processes implemented yet. 


- working: 
	- context switching 
	- idle process implementation
	- rudimentary clock handler

- not working:
	- block process 
	- delay not complete
	- clock interrupt handler not implemented


##### 2. Any other questions?

- taking an extention