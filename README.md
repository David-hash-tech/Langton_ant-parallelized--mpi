## Langton’s Ant

Obiective

The purpose of this project is to implement, in C, using the MPI library, a distributed and scalable program with the goal of simulating a modified version of the [Langton’s Ant](https://en.wikipedia.org/wiki/Langton%27s_ant) game invented by Chris Langton.

Introduction

The game consists of a map represented by a matrix of size H x W, with each element having one of two colors:

- White
- Black

The simulation will be executed for a given number of steps. On one of the elements of the matrix, an ant is placed and can move in one of the four cardinal directions at each step of the simulation, following the following rules:

- If it is on a white element, the ant turns 90° to the right (clockwise), changes the color of the square, and moves to the next square.

- If it is on a black element, the ant turns 90° to the left (counterclockwise), changes the color of the square, and moves to the next square.

However, in order to achieve one of the main objectives of this project, which is to implement distributed and scalable programs, we will consider multiple ants on the map. Thus, at most one ant will be instantiated on each element of the map. This implies that there may be multiple ants on the same element at any given time.

However, in order to avoid increasing the complexity level of the implementation too much, in case there are multiple ants on the same element, the color of the element will be changed only once.

Therefore, given the rules of movement and the fact that a square has four sides, this implies that after each step of the simulation, at most four ants can exist on the same element.

In addition, the map has a fixed (limited) size, so the ant can leave the area, which will remove it from the game..

Example simulation on a map of size 11 x 11:

![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.002.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.003.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.004.png)

(a) Initial map  (b) After step 1   (c) After step 2

![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.005.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.006.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.007.png)

(d) After step 3  (e) After step 4   (f) After step 5

![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.008.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.009.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.010.png)

(g) After step 6  (h) After step 7   (i) After step 8

![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.011.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.012.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.013.png)

(j) After step 9  (k) After step 10   (l) After step 11

![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.014.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.015.png) ![](Img/Aspose.Words.c228c6cd-f4b1-4828-bfd5-832dc8c94403.016.png)

(m) After step 12  (n) After step 13   (o) After step 14

The Simulation Process

The simulation will consist of a number of steps, with the state of the cells changing 'simultaneously' based on the cells in the previous step.

The program will take as input a file and a number of steps, representing the number of simulation steps, and the final result will be written to an output file.

On the first line, we will have:

- An integer H indicating the height of the map;
- An integer W indicating the width of the map;
- An integer S indicating the total number of steps to be performed in the simulation. This only exists in the input file;

Following this, there will be H lines with W columns, with each cell having a value represented by the concatenation of the elements COLOR and, optionally, ANT, where:

- COLOR has a value of 1 (WHITE) or 0 (BLACK);
- ANT has a value of 0 (ant facing LEFT), 1 (ant facing UP), 2 (ant facing RIGHT), 3 (ant facing DOWN), or may not exist.

Example input file (1) &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   Example output file (1)

5 5 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   5 5

1 1 1 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 1 1 1 1

1 1 12 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 1 01 1 1

1 11 10 13 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 0 02031 0 1

1 1 10 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 1 0 1 1

1 1 1 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 1 1 1 1


Example input file (2) &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   Example output file (2)

5 5 17 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   5 5

1 1 1 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 0 0 0 0

1 1 12 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 0 0 0 0

1 11 10 13 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   0 0 0 0 1

1 1 10 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   1 0 1 0 0

1 1 1 1 1 &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;   0 1 00 0 0

Running the program

The program can be run as follows:

1 mpirun --oversubscribe -np NUM_PROCS ./file.c IN_FILENAME OUT_FILENAME

Where:

- NUM~~ PROCS - the number of processes;
- IN~~ FILENAME - the name (path) of the input file;

- OUT~~ FILENAME - the name (path) of the output file;

Example:

1 mpirun --oversubscribe -np 2 ./homework in.txt out.txt
