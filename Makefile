#
#	Sample Makefile for Yalnix kernel and user programs.
#	
#	Prepared by Sean Smith and Adam Salem and various Yalnix developers
#	of years past...
#
#	You must modify the KERNEL_SRCS and KERNEL_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix kernel.
#
#	You must modify the USER_SRCS and USER_OBJS definition below to be your own
#	list of .c and .o files that should be linked to build your Yalnix user programs
#
#	The Yalnix kernel built will be named "yalnix".  *ALL* kernel
#	Makefiles for this lab must have a "yalnix" rule in them, and
#	must produce a kernel executable named "yalnix" -- we will run
#	your Makefile and will grade the resulting executable
#	named "yalnix".
#

#make all will make all the kernel objects and user objects
ALL = $(KERNEL_ALL) $(USER_APPS)
KERNEL_ALL = yalnix

#List all kernel source files here.  
KERNEL_SRCS = $(shell find src -type f -name '*.$(c)')
#src/list.c src/kernel.c src/pcb.c src/interupts.c src/loadprog.c src/syscalls.c
#List the objects to be formed form the kernel source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
KERNEL_OBJS = src/list.o src/kernel.o src/pcb.o src/interupts.o src/loadprog.o src/syscalls.o src/pipe.o src/tty.o src/lock.o src/cvar.o src/kernel_utils.o
#$(patsubst %.o,%.c,$(KERNEL_SRCS))
#List all of the header files necessary for your kernel
KERNEL_INCS = $(shell find include -type f -name '*.$(h)')
#include/list.h include/globals.h ,include/kernel.h include/pcb.h include/interupts.h include/loadprog.h include/syscalls.h


#List all user programs here.
USER_APPS = userland/fork userland/init userland/wait test/fork_test test/wait_test test/exec_test userland/pipe userland/tty userland/lock userland/simple userland/brk userland/cvar userland/cvar_b test/zero test/torture test/bigstack test/forktest test/forkbreak
#List all user program source files here.  SHould be the same as the previous list, with ".c" added to each file
USER_SRCS = userland/fork.c userland/init.c userland/wait.c test/fork_test.c test/wait_test.c test/exec_test.c userland/pipe.c userland/tty.c userland/lock.c userland/simple.c userland/brk.c userland/cvar.c userland/cvar_b.c test/zero.c test/torture.c test/bigstack.c test/forktest.c test/forkbreak.c

#List the objects to be formed form the user  source files here.  Should be the same as the prvious list, replacing ".c" with ".o"
USER_OBJS = test/fork_test.o userland/fork.o userland/init.o userland/wait.o test/fork_test.o test/wait_test.o test/exec_test.o userland/pipe.o userland/tty.o userland/lock.o userland/simple.o userland/brk.o userland/cvar.o userland/cvar_b.o test/zero.o test/torture.o test/forktest.o test/bigstack.o test/forkbreak.o


#List all of the header files necessary for your user programs
USER_INCS = 

#write to output program yalnix
YALNIX_OUTPUT = yalnix

#
#	These definitions affect how your kernel is compiled and linked.
#       The kernel requires -DLINUX, to 
#	to add something like -g here, that's OK.
#

#Set additional parameters.  Students generally should not have to change this next section

#Use the gcc compiler for compiling and linking
CC = gcc

DDIR58 = /yalnix
LIBDIR = $(DDIR58)/lib
INCDIR = $(DDIR58)/include
ETCDIR = $(DDIR58)/etc

# any extra loading flags...
LD_EXTRA = 

KERNEL_LIBS = $(LIBDIR)/libkernel.a $(LIBDIR)/libhardware.so

# the "kernel.x" argument tells the loader to use the memory layout
# in the kernel.x file..
KERNEL_LDFLAGS = $(LD_EXTRA) -L$(LIBDIR) -lkernel -lelf -Wl,-T,$(ETCDIR)/kernel.x -Wl,-R$(LIBDIR) -lhardware
LINK_KERNEL = $(LINK.c)

#
#	These definitions affect how your Yalnix user programs are
#	compiled and linked.  Use these flags *only* when linking a
#	Yalnix user program.
#

USER_LIBS = $(LIBDIR)/libuser.a
ASFLAGS = -D__ASM__
CPPFLAGS= -m32 -fno-builtin -I. -I$(INCDIR) -I include/ -g -DLINUX 


##########################
#Targets for different makes
# all: make all changed components (default)
# clean: remove all output (.o files, temp files, LOG files, TRACE, and yalnix)
# count: count and give info on source files
# list: list all c files and header files in current directory
# kill: close tty windows.  Useful if program crashes without closing tty windows.
# $(KERNEL_ALL): compile and link kernel files
# $(USER_ALL): compile and link user files
# %.o: %.c: rules for setting up dependencies.  Don't use this directly
# %: %.o: rules for setting up dependencies.  Don't use this directly

all: $(ALL)	

clean:
	rm -f bin/*~ bin/TTYLOG* bin/TRACE bin/$(YALNIX_OUTPUT) bin/core.* bin/DISK
cleaner:
	rm -f bin/* src/*.o userland/*.o 
	
count:
	wc $(KERNEL_SRCS) $(USER_SRCS)

list:
	ls -l *.c *.h

kill:
	killall yalnixtty yalnixnet yalnix

no-core:
	rm -f core.*

$(KERNEL_ALL): $(KERNEL_OBJS) $(KERNEL_LIBS) $(KERNEL_INCS)
	$(LINK_KERNEL) -o bin/$@ $(KERNEL_OBJS) $(KERNEL_LDFLAGS)
	rm -f src/*.o

$(USER_APPS): $(USER_OBJS) $(USER_INCS)
	$(ETCDIR)/yuserbuild.sh $@ $(DDIR58) $@.o
	mv $(@) ./bin
	rm -f $@.o




