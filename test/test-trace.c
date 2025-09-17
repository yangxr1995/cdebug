#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>

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
    printf("%s %d\n", __func__, __LINE__);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    close(fd);
    func2();
}

void test_epoll() {
    int efd = epoll_create(32);
    if (efd < 0) {
        perror("epoll_create");
        return ;
    }
    int fds[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) < 0) {
        perror("socketpair");
        return;;
    }
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR | EPOLLET;
    event.data.fd = fds[0];
    epoll_ctl(efd, EPOLL_CTL_ADD, fds[0], &event);

    printf("EPOLL_CTL_MOD : %d, EPOLL_CTL_DEL: %d\n", EPOLL_CTL_MOD, EPOLL_CTL_DEL);
    epoll_ctl(efd, EPOLL_CTL_MOD, fds[0], &event);

    int fd = open("/dev/stdin", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return ;
    }
    struct epoll_event event2;
    event2.events = EPOLLIN | EPOLLERR | EPOLLET;
    event2.data.fd = fd;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event2);

    struct epoll_event events[32];
    int cnt = epoll_wait(efd, events, sizeof(events)/sizeof(*events), -1);
    if (cnt < 0) {
        perror("epoll_wait");
        return ;
    }
}

void *start_thread(void *arg)
{
    func1();
    printf("%s %d\n", __func__, __LINE__);
    return NULL;
}

void test_pthread() {
    pthread_t tid;
    pthread_create(&tid, NULL, start_thread, NULL);
    while (1) {
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    // func1();
    // test_epoll();
    test_pthread();
    // pid_t pid;
    // pid = fork();
    // if (pid == 0) {
    //     printf("------1\n");
    //     func2();
    //     func3();
    // }
    // else {
    //     printf("------2\n");
    //     func1();
    // }

    return EXIT_SUCCESS;
}
