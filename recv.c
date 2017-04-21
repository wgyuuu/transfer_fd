#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include "nsocket.h"

void check_sock(int);

int main(void)
{
    dup(STDIN_FILENO);

    int server_fd = socket_unix("/tmp/fd.sock");
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
        }

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

    char data[] = "wo ying le.";
    send(cli_fd, data, sizeof(data), MSG_OOB);

    return 0;
}

