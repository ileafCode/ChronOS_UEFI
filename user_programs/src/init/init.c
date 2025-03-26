
#include <syscall/syscall.h>

int main(int argc, char **argv) {
    int fd = syscall(3, (uint64_t)("/init"), 0x03, 0, 0, 0, 0);
    syscall(4, fd, 0, 0, 0, 0, 0);
    syscall(6, 1, (uint64_t)("This is from INIT. Yay!"), 23, 0, 0, 0);
    return 0;
}