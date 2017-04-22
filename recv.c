#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <signal.h>

#include "nsocket.h"

void check_sock(int);
static void sig_process(int);

int main(void)
{
    if (signal(SIGINT, sig_process) == SIG_ERR)
        printf("signal error.\n");

    dup(STDIN_FILENO);

    const char *unix_path = "/tmp/fd.sock";
    unlink(unix_path);
    int server_fd = socket_unix(unix_path);
    printf("server_fd:%d.\n", server_fd);

    int cli_fd;

    while (1)
    {
        cli_fd = accept(server_fd, NULL, NULL);
        if (cli_fd == -1)
        {
            printf("accept error, err:%d.\n", errno);
            exit(1);
        }

        int read_fd;

        union {
            struct cmsghdr cm;
            char space[CMSG_SPACE(sizeof(int))];
        } cmsg;

        memset(&cmsg, 0, sizeof(cmsg));

        char buf[2];
        struct iovec iov[1];
        iov[0].iov_base = buf;
        iov[0].iov_len = sizeof(buf);

        struct msghdr msg;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_control = &cmsg;
        msg.msg_controllen = sizeof(cmsg);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        size_t n;
        n = recvmsg(cli_fd, &msg, 0);
        if (n < 0)
        {
            printf("recv fd:%d,err:%d\n", cli_fd, errno);
            continue;
        } else 
        {
            printf("recvmsg: %s.\n", buf);
            char *recv_buf = "recv ok!";
            n = send(cli_fd, recv_buf, strlen(recv_buf), 0);
            if (n == -1)
            {
                printf("send to client error, err:%d strerrno:%s.\n", errno, strerror(errno));
            }
        }

        close(cli_fd);

        memcpy(&read_fd, CMSG_DATA(&cmsg.cm), sizeof(int));
        printf("read_fd:%d\n", read_fd);

        if (read_fd > 0)
            check_sock(read_fd);
    }
}


void check_sock(int serv_fd) 
{
    struct sockaddr_in sockaddr_cli;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int cli_fd = accept(serv_fd, &sockaddr_cli, &addr_len);
    if (cli_fd == -1)
    {
        printf("accept2 error, err:%d.\n", errno);
        exit(1);
    }

    char data[] = "wo ying le.\n";
    // MSG_OOB out-of-band 写入一个单子节到下一个包，使其成为紧急数据
    int n = send(cli_fd, data, sizeof(data), 0);
    if (n == -1)
    {
        printf("send to client error, err:%d.\n", errno);
        return;
    }

    printf("send to client %d bytes.\n", n);
    close(cli_fd);

    close(serv_fd);
}


void sig_process(int signo)
{
    printf("signal number: %d.\n", signo);
    exit(1);
}
