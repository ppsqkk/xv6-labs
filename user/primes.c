#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve();
int read_int(int fd, int *n);
int write_int(int fd, int n);

int main()
{
    dup(1); // 3 is stdout now

    int p[2];
    pipe(p);
    if (fork() == 0) {
        // Child
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        sieve();
    }
    else {
        // Parent
        close(p[0]);
        for (int N = 2; N <= 35; N++) {
            write_int(p[1], N); // Assume no error
        }
        close(p[1]);
        wait(0);
    }
    exit(0);
}

// Requires left pipe read to be available at fd 0
// Requires no references to left pipe write
void sieve()
{
    int prime;
    if (read_int(0, &prime) == 0) {
        // Base case: no primes left
        close(0);
        return;
    }
    fprintf(3, "prime %d\n", prime);

    int p[2];
    pipe(p);
    if (fork() == 0) {
        // Child
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        sieve();
    }
    else {
        // Parent
        close(p[0]);
        int candidate;
        while (read_int(0, &candidate) != 0) {
            if (candidate % prime != 0) {
                write_int(p[1], candidate);
            }
        }
        close(0);
        close(p[1]);
        wait(0);
    }
}

int read_int(int fd, int *n)
{
    char buf[4];
    int bytes_read = read(fd, buf, 4);
    *n = *(int *) buf;
    return bytes_read;
}

int write_int(int fd, int n)
{
    return write(fd, (char *) &n, 4);
}
