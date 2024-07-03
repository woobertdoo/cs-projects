#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHMKEY 859047
#define BUFF_SIZE 2 * sizeof(int)

void parent();
void child(int argc, char **argv);

void help() {
    printf("This function utilizes shared memory!\n");
    printf("It takes a set of positive integer arguments and outputs the first "
           "pair of arguments that sum to 19\n");
}

int main(int argc, char **argv) {

    if (argc == 1) {
        printf("Please enter arguments!\n");
        help();
        return 1;
    }

    int option;

    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
        case 'h':
            help();
            return 1;
        case '?':
            if (isprint(optopt)) {
                fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                return 1;
            } else {
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                return 1;
            }
        }
    }

    int shmid = shmget(SHMKEY, BUFF_SIZE, 0777 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error in shared memory creation");
        exit(1);
    }
    int *starting = (int *)shmat(shmid, 0, 0);
    *starting = -200;
    *(starting + sizeof(int)) = -200;

    switch (fork()) {
    case -1:
        perror("Failed to fork process\n");
        return 1;
    case 0:
        child(argc, argv);
        break;
    default:
        parent();
        break;
    }

    return 0;
}

void child(int argc, char **argv) {

    int i, j, a = -1, b = -1;
    int shmid = shmget(SHMKEY, BUFF_SIZE, 0777);

    for (i = 1; i < argc - 1; i++) {
        for (j = i + 1; j < argc; j++) {
            a = atoi(argv[i]);
            b = atoi(argv[j]);
            if (a < 0 || b < 0) {
                fprintf(stderr,
                        "Only enter positive numbers! Error at pair %d %d\n", a,
                        b);
                exit(1);
            }
            if (a + b == 19)
                goto write;
            else {
                a = -1;
                b = -1;
            }
        }
    }
write:
    if (shmid == -1) {
        perror("Child: Error in getting shared memory");
        exit(1);
    }

    int *cint = (int *)shmat(shmid, 0, 0);

    if (*cint != -200 || *(cint + sizeof(int)) != -200) {
        perror("Shared memory error.\n");
        exit(1);
    }

    *cint = a;
    *(cint + sizeof(int)) = b;
}

void parent() {
    int status;
    wait(&status);
    if (status != 0) {
        fprintf(stderr,
                "Error in child status (exited with %d). Terminating..\n",
                status);
    }

    int shmid = shmget(SHMKEY, BUFF_SIZE, 0777 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error getting shared memory");
    }

    int *pint = (int *)shmat(shmid, 0, 0);
    if (*pint == -200 && *(pint + sizeof(int)) == -200) {
        perror("Error occured during shared memory creation");
        exit(1);
    } else if (*pint == -1 && *(pint + sizeof(int)) == -1) {
        printf("No pairs that sum to 19 found");
    } else {
        printf("Pair found in child: %d %d\n", *pint, *(pint + sizeof(int)));
    }

    shmdt(pint);
    shmctl(shmid, IPC_RMID, NULL);

    return;
}
