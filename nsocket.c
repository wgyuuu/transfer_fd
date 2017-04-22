#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

int socket_tcp(uint16_t port)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htons(INADDR_ANY);

    int reuse = 1, alive = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(int));
    // setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&alive, sizeof(int));

    if (bind(sockfd, &sockaddr, sizeof(struct sockaddr_in)) == -1)
    {
        printf("binding error, err:%d.\n", errno);
        exit(1);
    }

    int backlog = 128;
    if (listen(sockfd, backlog) == -1)
    {
        printf("listen error, err:%d.\n", errno);
        exit(1);
    }

    return sockfd;
}

int conn_tcp(const char *addr, uint16_t port)
{
    int cli_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sockaddr_cli;
    sockaddr_cli.sin_family = AF_INET;
    sockaddr_cli.sin_addr.s_addr = inet_addr(addr);
    sockaddr_cli.sin_port = htons(port);

    int err = connect(cli_fd, &sockaddr_cli, sizeof(struct sockaddr_in));
    if (err == -1)
    {
        printf("connect error, cli_fd:%d, err:%d\n", cli_fd, errno);
        exit(1);
    }

    return cli_fd;
}

int socket_unix(const char *path) 
{
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        printf("create socket error, err:%d.\n", errno);
        exit(1);
    }

    // int recv_size = 16;
    // setsockopt(listen_fd, SOL_SOCKET, SO_RCVLOWAT, &recv_size, sizeof(int));

    struct sockaddr_un saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, path);

    int status;
    status = bind(listen_fd, &saddr, SUN_LEN(&saddr));
    if (status == -1)
    {
        printf("bind error, err:%d.\n", status);
        exit(1);
    }

    int backlog = 128;
    status = listen(listen_fd, backlog);
    if (status == -1)
    {
        printf("listen error, err:%d.\n", errno);
        exit(1);
    }

    return listen_fd;
}

int conn_unix(const char *path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        printf("create socket error, err:%d.\n", errno);
        exit(1);
    }

    struct sockaddr_un saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, path);

    int status = connect(fd, &saddr, SUN_LEN(&saddr));
    if (status == -1)
    {
        printf("connect error, err:%d.\n", errno);
        exit(1);
    }

    return fd;
}
