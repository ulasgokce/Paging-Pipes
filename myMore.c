#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

#define BUFFER_SIZE 500
#define MAXIMUM_LINE_NUMBER 100
#define LINES_PER_TURN 24

int main(int argc, char *argv[])
{

    int read_exit = atoi(argv[1]);
    char read_msg[MAXIMUM_LINE_NUMBER][BUFFER_SIZE];

    read(read_exit, &read_msg, LINES_PER_TURN * BUFFER_SIZE);

    for (int i = 0; i < LINES_PER_TURN; ++i)
    {
        printf("%s", read_msg[i]);
    }

    close(read_exit);
    return 0;
}
