/*
 * =====================================================================================
 *
 *       Filename:  send_fd.c
 *
 *    Description:  尝试一下进程间fd的传递
 *
 *        Version:  1.0
 *        Created:  2017/04/18 11时51分26秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  wgyuuu (),
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "nsocket.h"

void send_fd(int);

int main(void)
{
    int fd = socket_tcp(8888);

    send_fd(fd);
}

void send_fd(int fd)
{

    struct msghdr msg;

    union
    {
        struct cmsghdr cm;
        char space[CMSG_SPACE(sizeof(int))];
    } cmsg;

    memset(&cmsg, 0, sizeof(cmsg));

    cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
    cmsg.cm.cmsg_level = SOL_SOCKET;
    cmsg.cm.cmsg_type = SCM_RIGHTS;
    memcpy(CMSG_DATA(&cmsg.cm), &fd, sizeof(int));

    char buf[2] = "ab";
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    int cli_fd = conn_unix("/tmp/fd.sock");

    int n;
    // n = send(cli_fd, "aaaaa", 5, MSG_OOB);
    n = sendmsg(cli_fd, &msg, 0);
    if (n == -1) {
        printf("send error, cli_fd:%d, err:%d\n", cli_fd, errno);
        exit(1);
    } else {
	    printf("send count bytes:%d.\n", n);
    }

    char *recv_buf[8];
    n = recv(cli_fd, recv_buf, sizeof(recv_buf), 0);
    if (n < 0)
    {
        printf("recv error, err:%d.\n", errno);
    } else {
        printf("recv: %s.\n", recv_buf);
    }

    close(cli_fd);
}
