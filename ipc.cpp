#include "ipc.h"

struct socket_t ss;
int sock_fd;

int create_socket(int sockfd, struct ipc_msg *msg) {
    struct ipc_socket *sock = reinterpret_cast<ipc_socket *>(msg->data);
    ss.domain = sock->domain;
    ss.pid = msg->pid;
    ss.protocol = sock->protocol;
    ss.type = sock->type;

    int resplen = sizeof(struct ipc_msg) + sizeof(struct ipc_err);
    struct ipc_msg *response = (ipc_msg *) alloca(resplen);

    response->type = IPC_SOCKET;
    response->pid = msg->pid;

    struct ipc_err err;
    err.err = 0;
    err.rc = 8080;
    memcpy(response->data, &err, sizeof(struct ipc_err));
//    ipc_try_send(sockfd, (char *)response, resplen)
//    send(sockfd, buf, len, MSG_NOSIGNAL)
//    send(sockfd, buf, len, MSG_NOSIGNAL)
//sendto(fd, buf, len,
//                                        flags, dest_addr, dest_len)
    int res = sendto(sockfd, response, resplen,
                     MSG_NOSIGNAL, NULL, 0);
//    int res = write(sock_fd, response, resplen);
    return res;
}

void do_connect(int sockfd, struct ipc_msg *msg) {
    struct ipc_connect *payload = (struct ipc_connect *)msg->data;
    pid_t pid = msg->pid;
    auto a = payload->addr.sa_data;
    int rc = -1;
}

void process_socket(int fd, int datasock) {
    int rc;
    int LEN = 8192;
    char buf[LEN];
    sock_fd = datasock;

    while ((rc = read(fd, buf, LEN)) > 0) {
        std::cout << "receive buf" << rc << buf << std::endl;
        struct ipc_msg *msg = reinterpret_cast<ipc_msg *>(buf);
        std::cout << "the type of message of " << msg->type;
        switch (msg->type) {
            case IPC_SOCKET:
                create_socket(fd, msg);
                break;
            case IPC_CONNECT:
                do_connect(fd, msg);
                break;
        }
    };
}

void init_socket() {
    char const *sockname = "/tmp/lvlip.socket";
    unlink(sockname);

    int datasock, fd;
    if ((datasock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sockname, sizeof(addr.sun_path) - 1);

    int rc = bind(datasock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr.sun_path) - 1);
    if (rc == -1) {
        perror("bind error.");
        exit(EXIT_FAILURE);
    }
    rc = listen(datasock, 20);
    if (rc == -1) {
        perror("listen error.");
        exit(EXIT_FAILURE);
    }
    if (chmod(sockname, S_IRUSR | S_IWUSR | S_IXUSR |
                        S_IRGRP | S_IWGRP | S_IXGRP |
                        S_IROTH | S_IWOTH | S_IXOTH) == -1) {
        perror("Chmod on lvl-ip IPC UNIX socket failed");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        fd = accept(datasock, NULL, NULL);
        if (fd == -1) {
            perror("accpet error");
            exit(EXIT_FAILURE);
        }
        process_socket(fd, datasock);
    }

    close(fd);
    unlink(sockname);
}

void rx_loop() {
    init_socket();
}