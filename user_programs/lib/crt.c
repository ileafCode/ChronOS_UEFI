
#include <syscall/syscall.h>

extern int main(int, char **);

void _start() {
    int x = main(0, (char **)0);
    while (1);
}