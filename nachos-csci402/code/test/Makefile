# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles testPageTable testFork testExec testLock testCondition passportOffice twoPassportOffices

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

testPageTable.o: testPageTable.c
	$(CC) $(CFLAGS) -c testPageTable.c
testPageTable: testPageTable.o start.o
	$(LD) $(LDFLAGS) start.o testPageTable.o -o testPageTable.coff
	../bin/coff2noff testPageTable.coff testPageTable

testFork.o: testFork.c
	$(CC) $(CFLAGS) -c testFork.c
testFork: testFork.o start.o
	$(LD) $(LDFLAGS) start.o testFork.o -o testFork.coff
	../bin/coff2noff testFork.coff testFork

testExec.o: testExec.c
	$(CC) $(CFLAGS) -c testExec.c
testExec: testExec.o start.o
	$(LD) $(LDFLAGS) start.o testExec.o -o testExec.coff
	../bin/coff2noff testExec.coff testExec

testLock.o: testLock.c
	$(CC) $(CFLAGS) -c testLock.c
testLock: testLock.o start.o
	$(LD) $(LDFLAGS) start.o testLock.o -o testLock.coff
	../bin/coff2noff testLock.coff testLock

testCondition.o: testCondition.c
	$(CC) $(CFLAGS) -c testCondition.c
testCondition: testCondition.o start.o
	$(LD) $(LDFLAGS) start.o testCondition.o -o testCondition.coff
	../bin/coff2noff testCondition.coff testCondition

passportOffice.o: passportOffice.c
	$(CC) $(CFLAGS) -c passportOffice.c
passportOffice: passportOffice.o start.o
	$(LD) $(LDFLAGS) start.o passportOffice.o -o passportOffice.coff
	../bin/coff2noff passportOffice.coff passportOffice

twoPassportOffices.o: twoPassportOffices.c
	$(CC) $(CFLAGS) -c twoPassportOffices.c
twoPassportOffices: twoPassportOffices.o start.o
	$(LD) $(LDFLAGS) start.o twoPassportOffices.o -o twoPassportOffices.coff
	../bin/coff2noff twoPassportOffices.coff twoPassportOffices
clean:
	rm -f *.o *.coff
