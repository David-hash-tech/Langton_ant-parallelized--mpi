#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define ALB 1
#define NEGRU 0

#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3

#define MAXLENGTH 5
#define GO_LEFT(a) a = a - 1
#define GO_UP(a, W) a = a - W
#define GO_RIGHT(a) a = a + 1
#define GO_DOWN(a, W) a = a + W

typedef struct Ant
{
    int direction;
    int position;
} Ant;

void readInputFile(const char *filename, int *rows, int *columns, int *steps, int **map, int nProcesses)
{
    char str1[10];
    char str2[10];
    FILE *in = fopen(filename, "r");

    fscanf(in, "%d %d %d\n", rows, columns, steps);

    int nrLinesPad = (*rows % nProcesses == 0) ? 0 : nProcesses - (*rows % nProcesses);
    (*map) = (int *)malloc((*rows + nrLinesPad) * (*columns) * sizeof(int));

    for (int i = 0; i < (*rows); i++)
        for (int j = 0; j < (*columns); j++)
        {
            fscanf(in, "%s", str2);
            strcpy(str1, "1");
            strcat(str1, str2);
            (*map)[i * (*columns) + j] = (int)strtol(str1, (char **)NULL, 10);
        }

    for (int i = (*rows); i < (*rows) + nrLinesPad; i++)
        for (int j = 0; j < (*columns); j++)
        {
            (*map)[i * (*columns) + j] = 15;
        }

    fclose(in);
    return;
}

int getDataFromElement(int *data, int element)
{
    int index = 0;
    while (element)
    {
        (data)[index] = element % 10;
        element = element / 10;
        index++;
    }

    return index;
}

int calculateDirectionOfAnt(int color, int actualDirection)
{
    if (color == ALB)
    {
        return (actualDirection + 1) % 4;
    }
    else if (color == NEGRU)
    {
        return (actualDirection + 3) % 4;
    }

    return -1;
}

int calculatePositionInVector(int rank, int directionOfAnt, int actualPositionInVector, int nrElementsInVector, int W)
{
    switch (directionOfAnt)
    {
    case UP:
    {
        return GO_UP(actualPositionInVector, W);
    }
    case DOWN:
    {
        return GO_DOWN(actualPositionInVector, W);
    }
    case LEFT:
    {
        return GO_LEFT(actualPositionInVector);
    }
    case RIGHT:
    {
        return GO_RIGHT(actualPositionInVector);
    }
    }

    return -1;
}

void addAntToVector(int *index, int **vectorAnts, int newPosition)
{
    (*vectorAnts)[(*index)++] = newPosition;
    return;
}

void addAntToStructureVector(int *index, Ant **vectorAnts, int newPosition, int newDirection)
{
    (*vectorAnts)[(*index)].position = newPosition;
    (*vectorAnts)[(*index)].direction = newDirection;
    (*index)++;

    return;
}

void sendData(int rank, int nProcesses, int UpIndex, int *UpAnts, int DownIndex, int *DownAnts)
{
    if (rank == 0)
    {
        MPI_Send(&DownIndex, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);

        if (DownIndex != 0)
            MPI_Send(DownAnts, DownIndex, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
    else if (rank == nProcesses - 1)
    {
        MPI_Send(&UpIndex, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

        if (UpIndex != 0)
            MPI_Send(UpAnts, UpIndex, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send(&UpIndex, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

        if (UpIndex != 0)
            MPI_Send(UpAnts, UpIndex, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

        MPI_Send(&DownIndex, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);

        if (DownIndex != 0)
            MPI_Send(DownAnts, DownIndex, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    return;
}

void receiveDataAndUpdate(int rankToReceiveFrom, int **minimap, int antDirection)
{
    int NrRes = -1;

    MPI_Recv(&NrRes, 1, MPI_INT, rankToReceiveFrom, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (NrRes != 0)
    {
        int *res = (int *)malloc(NrRes * sizeof(int));

        MPI_Recv(res, NrRes, MPI_INT, rankToReceiveFrom, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // update the vector with the new elements; careful if the position is pad
        for (int i = 0; i < NrRes; i++)
            (*minimap)[res[i]] = (*minimap)[res[i]] * 10 + antDirection;
    }

    return;
}

void manageReceive(int rank, int nProcesses, int **minimap)
{
    if (rank == 0)
    {
        receiveDataAndUpdate(rank + 1, minimap, UP);
    }
    else if (rank == nProcesses - 1)
    {
        receiveDataAndUpdate(rank - 1, minimap, DOWN);
    }
    else
    {
        receiveDataAndUpdate(rank + 1, minimap, UP);
        receiveDataAndUpdate(rank - 1, minimap, DOWN);
    }

    return;
}

void updateLeftAndRightAnts(int RightIndex, Ant *RightAnts, int **minimap, int nrElements)
{
    for (int i = 0; i < RightIndex; i++)
    {
        (*minimap)[RightAnts[i].position] = (*minimap)[RightAnts[i].position] * 10 + RightAnts[i].direction;
    }

    return;
}

void writeOutputFile(const char *filename, int rows, int columns, int *map)
{
    char str[10];
    FILE *out = fopen(filename, "w");

    fprintf(out, "%d %d\n", rows, columns);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            sprintf(str, "%d", map[i * columns + j]);
            fprintf(out, "%s ", str + 1);
        }
        fprintf(out, "\n");
    }

    fclose(out);
    return;
}

int main(int argc, char **argv)
{
    int rank, nProcesses;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    int H, W, S;
    int *vector = NULL;
    int *minimap = NULL;

    if (rank == 0)
    {
        readInputFile(argv[1], &H, &W, &S, &vector, nProcesses);
    }

    MPI_Bcast(&H, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&W, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&S, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int nrLinesPad = (H % nProcesses == 0) ? 0 : nProcesses - (H % nProcesses);
    int lines_per_proc = (H + nrLinesPad) / nProcesses;
    int nrElements = lines_per_proc * W;

    minimap = (int *)malloc(nrElements * sizeof(int));

    MPI_Scatter(vector, nrElements, MPI_INT, minimap, nrElements, MPI_INT, 0, MPI_COMM_WORLD);

    int color = -1;
    int noAntsOnPosition = -1;
    int direction = -1;
    int positionInAnotherProcess = -1;
    int position = -1;

    int UpIndex = 0;
    int *UpAnts = (int *)malloc(W * sizeof(int));

    int DownIndex = 0;
    int *DownAnts = (int *)malloc(W * sizeof(int));

    int RightIndex = 0;
    Ant *RightAnts = (Ant *)malloc(W * lines_per_proc * sizeof(Ant));

    int data[MAXLENGTH];

    while (S != 0)
    {
        //   for each position in the vector
        for (int i = 0; i < nrElements; i++)
        {
            if (minimap[i] < 100)
                continue;

            noAntsOnPosition = getDataFromElement(data, minimap[i]);
            color = data[noAntsOnPosition - 2];

            // for each ant
            for (int j = 0; j < noAntsOnPosition - 2; j++)
            {
                direction = calculateDirectionOfAnt(color, data[j]);
                position = calculatePositionInVector(rank, direction, i, nrElements, W);

                if (position < 0)
                {
                    if (i == 0 && direction == LEFT)
                        continue;

                    if (rank == 0) // if the ant leaves the matrix through top
                        continue;

                    positionInAnotherProcess = nrElements + position;

                    addAntToVector(&UpIndex, &UpAnts, positionInAnotherProcess);
                    // save direction of ant in a vector in order to send it to the process with rank - 1
                }
                else if (position >= nrElements)
                {
                    if (i == nrElements - 1 && direction == RIGHT)
                        continue;

                    positionInAnotherProcess = position - nrElements;

                    addAntToVector(&DownIndex, &DownAnts, positionInAnotherProcess);
                    // save direction of ant in a vector in order to send it to the process with rank - 1
                }
                else
                {
                    if (position < i)
                    {
                        if (direction == LEFT && ((i % W) == 0))
                            continue;

                        minimap[position] = minimap[position] * 10 + direction;
                    }

                    if (position > i)
                    {
                        if (direction == RIGHT && ((i + 1) % W == 0)) // leave the line of the matrix
                            continue;

                        if (minimap[position] == 15) // position is on pad
                            continue;

                        addAntToStructureVector(&RightIndex, &RightAnts, position, direction);
                    }
                }
            }

            // change color; be mindful of format of the vector's elements
            minimap[i] = 10 + ((color + 1) % 2);
        }

        if (nProcesses > 1)
        {
            sendData(rank, nProcesses, UpIndex, UpAnts, DownIndex, DownAnts);
            manageReceive(rank, nProcesses, &minimap);
        }

        updateLeftAndRightAnts(RightIndex, RightAnts, &minimap, nrElements);

        UpIndex = 0;
        DownIndex = 0;
        RightIndex = 0;

        S--;
    }

    MPI_Gather(minimap, nrElements, MPI_INT, vector, nrElements, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        writeOutputFile(argv[2], H, W, vector);
    }

    MPI_Finalize();
    return 0;
}
