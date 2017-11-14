# Yalnix kernel, COSC 58, 17F
### Group: ‘54 65 61 6D’; Drop Tables; team
Members: Patrick J. Flathers (pjflathers) and Bruno Korbar (bjuncek)

This is a part of a our operating system. Our operating system is the greatest operating system. It runs programs
![](http://folk.uio.no/hpv/linuxtoons/foxtrot.2003-08-14.gif)


---



### Design and documentation

##### Checkpoints
0. Checkpoint one completed on Oct 8th, 2017. *3/5*
1. Checkpoint two *5/5*
2. Checkpoint three *4/5*
3. Checkpoint four *4/5*
4. Checkpoint five *4/5*

So far everything we tried worked. Thus we are cool.

### Usage manual and description

#### Makefile and compilation:
Run `make` in the yalnix virtual machine. Makefile is in the root of the repository, and it is expected to build all
related programs in `.\bin` directory. 
Additional makefile commands are (as described in the manual):
0. `make clean`
1. `make no-core`
2. `make count`
3. `make list`
4. `make kill`

#### To Run:
To run with the basic information
`yalnix -t tracefile -lk 4 -s <insert user program here>`
and to run it with extended debugging info, run 
`yalnix -t tracefile -lk 10 -s <insert user program here>`

##### Traceprint level guide:
0. Enter and exit for kernel start only
1. Enter and exit into interrupts; exit inducing errors;
2. Enter and exit into syscalls and further nested functions; Errors in them
3. Debugging messages: general information
6. Verbose debugging messages and in-progress goals


### Folder organisation
- **src**: source files
- **include**: header files, datastructure definitions
- **userland**: userland programs that illustrate how each part of our program would behave and work
- **sample**: provided by Sean, and templates
- **sample_include**: sample .h files for interaction with yalnix "hardware", provided by Sean


### Trivia:
1. when I first started to write this readme, I wanted to write it using [the top ten hundred words](https://www.xkcd.com/simplewriter/), however the closest thing to writing _kernel_ with it was _many letters that allow computer to run things on it and not break_.
2. While writing _fork_, Bruno wrote half of it using python syntax. He attributes that to the [lack of alchohol](https://xkcd.com/323/) involved in the proccess.
3. _kernel_Exit_ was rewritten from scratch for a total 4 times; _kernel_Fork_ was rewritten 3 times. The problem was actually in fork userland info
4. We discovered and abused `git blame`
5. Sean spent 4 hours finding 3 extra chars in the interrupt handler (git blame: Bruno) 
![true story](https://imgs.xkcd.com/comics/git_commit_2x.png)
![What we do](http://folk.uio.no/hpv/linuxtoons/foxtrot.1999-08-16.png)
