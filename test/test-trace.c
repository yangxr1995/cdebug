#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void func3() {
    char buf[32], str[32];
    int a = random();
    sprintf(str, "%d", a);
    strcpy(buf , str);
    printf("buf: %s\n", buf);
}

void func2() {
    int ret = access("./aaa", F_OK);
    func3();
}

void func1() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    close(fd);
    func2();
}
int main(int argc, char *argv[])
{
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        printf("------1\n");
        func2();
        func3();
    }
    else {
        printf("------2\n");
        func1();
    }

    return EXIT_SUCCESS;
}
