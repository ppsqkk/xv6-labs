#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    char buf[1];
    int p1[2], p2[2];

    pipe(p1);
    pipe(p2);
    if (fork() == 0) {
        // Child
        close(p1[1]);
        if (read(p1[0], buf, 1) > 0) {
            printf("%d: received ping\n", getpid());
        }
        write(p2[1], "c", 1);
    }
    else {
        // Parent
        close(p2[1]);
        write(p1[1], "p", 1);
        if (read(p2[0], buf, 1) > 0) {
            printf("%d: received pong\n", getpid());
        }
    }
    exit(0);
}
