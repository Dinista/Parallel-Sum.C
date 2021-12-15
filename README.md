# Parallel-Sum.C

## Introduction

This is a implementation of the parallel version of the program <i> sum.c </i>, wich is a master/worker task queue model of parallel computation.
The primary purpose of this project is to gain experience with multicore parallelism using the standard POSIX <a href="https://en.wikipedia.org/wiki/POSIX_Threads">Pthreads</a> library, including exposure to multithreading concepts such as <b>mutexes</b> and <b>conditions</b>.

An portuguese article was written analyzing the performance of the parallel implementation considering the <a href="https://en.wikipedia.org/wiki/Speedup">Speedup</a> metric. The article can be found in the <i>/docs</i> folder.

<b> Second project of the class Concurrent Programming. </b>

## How it works

The program reads a list of "tasks" from a file. Each task consists of a character code indicating an action and a number. The character code can be either a "p" (for "process") or "w" (for "wait"). The input file simulates various workloads entering a multiprocessing system.

In a real system, the "p" actions (the tasks) would likely be calls to computational routines. In this case, "processing" a task with number n just means waiting n seconds using the sleep function and then updating a few global aggregate variables (sum, odd count, min, and max). The "w" action provides a way to simulate a pause in incoming tasks.

### Input file

For example, in the following input simulates one initial one-second task entering the system, followed by a two-second delay.

After the delay, a two-second task enters the system followed by a three-second task.

```
p 1
w 2
p 2
p 3
```

### Output

The final output should match the following (sum, # odd, min, max):

```
6 2 1 3
```

## How to use

The <i>file.txt</i> must be in the same folder as the program.

Compile with make

```console
make
```
Run:

```console
./par_sum [-f <File name>] [-t <Number of threads>]
```

Example

```console
./par_sum -f test1 -t 4
```
