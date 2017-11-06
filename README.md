# Yalnix kernel, COSC 58, 17F
### Group: ‘54 65 61 6D’; Drop Tables; team
Members: Patrick J. Flathers (pjflathers) and Bruno Korbar (bjuncek)


![What we do](http://folk.uio.no/hpv/linuxtoons/foxtrot.1999-08-16.png)

---



### Design and documentation
Please find current state of the project in [General.todo](General.todo) file. 
 - Checkpoint one completed on Oct 8th, 2017. *3/5*
 - Checkpoint two *5/5*
 - Checkpoint three --> in progress

### Usage manual and description
#### To Compile:
Run `make` in the yalnix virtual machine. Makefile is in the rood of the repository, and it is expected to build all
related programs in `.\bin` directory. 
Additional makefile commands are (as described in the manual):
- `make clean`
- `make no-core`
- `make count`
- `make list`
- `make kill`

#### To Run:
`yalnix -t tracefile -lk 4 -s <insert user program here>`

##### Traceprint level guide:
0. Enter and exit for kernel start only
1. Enter and exit into every major function; exit inducing errors; running process printfs
2. Enter and exit into nested functions; Debugging messages: announcments
3. Debugging messages: general information
6. Verbose debugging

#### Trivia:
![true story](https://imgs.xkcd.com/comics/git_commit_2x.png)