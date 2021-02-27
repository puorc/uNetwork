#include "MessageListener.h"

void MessageListener::start() {
    unlink(unix_sock_path);
    int main_sock;
    if ((main_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        throw std::system_error(EBADF, std::system_category(), "Unable to open Unix socket.");
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, unix_sock_path, sizeof(addr.sun_path) - 1);

    int rc = bind(main_sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr.sun_path) - 1);
    if (rc == -1) {
        throw std::system_error(EBADF, std::system_category(), "Unable to bind to socket.");
    }
    if ((listen(main_sock, 20)) == -1) {
        throw std::system_error(EBADF, std::system_category(), "Unable to listen to socket.");
    }
    if (chmod(unix_sock_path, S_IRUSR | S_IWUSR | S_IXUSR |
                              S_IRGRP | S_IWGRP | S_IXGRP |
                              S_IROTH | S_IWOTH | S_IXOTH) == -1) {
        throw std::system_error(EBADF, std::system_category(), "Unable to set permission.");
    }
    for (;;) {
        int fd = accept(main_sock, NULL, NULL);
        if (fd == -1) {
            throw std::system_error(EBADF, std::system_category(), "Accept error");
        }
        std::thread t(&MessageListener::process, this, fd);
        t.detach();
    }
    close(main_sock);
}

void MessageListener::process(int fd) {
    int rc;
    char buf[LEN];
    while ((rc = read(fd, buf, LEN)) > 0) {
        auto *msg = reinterpret_cast<ipc_msg *>(buf);
        switch (msg->type) {
            case IPC_SOCKET:
                reply(fd, IPC_SOCKET, msg->pid, tcp.socket());
                break;
            case IPC_CONNECT: {
                net_connect(msg, fd);
                break;
            }
            case IPC_READ: {
                net_read(msg, fd);
                break;
            }
            case IPC_WRITE: {
                net_write(msg, fd);
                break;
            }
            case IPC_FCNTL: {
                struct ipc_fcntl *fc = (struct ipc_fcntl *) msg->data;
                pid_t pid = msg->pid;
                int rc = -1;

                switch (fc->cmd) {
                    case F_GETFL:
                        rc = O_RDWR;
                        break;
                    case F_SETFL:
                        rc = 0;
                        break;
                    default:
                        rc = -EINVAL;
                }

                reply(fd, IPC_FCNTL, pid, rc);
                break;
            }
            case IPC_POLL: {
                net_poll(msg, fd);
                break;
            }
            case IPC_CLOSE: {
                pid_t pid = msg->pid;
                auto *payload = (struct ipc_close *) msg->data;
                tcp.close(payload->sockfd);
                reply(fd, IPC_CLOSE, pid, 0);
                break;
            }
            case IPC_GETPEERNAME:
            case IPC_GETSOCKNAME:
                getname(fd, msg);
                break;
        }
    }
    close(fd);
}

void MessageListener::reply(int fd, int type, pid_t pid, int rc) {
    size_t size = sizeof(struct ipc_msg) + sizeof(struct ipc_err);
    auto *response = reinterpret_cast<ipc_msg *>(new uint8_t[size]);

    response->type = type;
    response->pid = pid;

    struct ipc_err err;

    if (rc < 0) {
        err.err = -rc;
        err.rc = -1;
    } else {
        err.err = 0;
        err.rc = rc;
    }

    memcpy(response->data, &err, sizeof(struct ipc_err));

    if (send(fd, response, size, MSG_NOSIGNAL) == -1) {
        perror("Error on writing IPC write response");
    }
}

void MessageListener::getname(int fd, struct ipc_msg *msg) {
    pid_t pid = msg->pid;
    auto *name = reinterpret_cast<ipc_sockname *>(msg->data);

    int rc = 0;

    size_t resplen = sizeof(struct ipc_msg) + sizeof(struct ipc_err) + sizeof(struct ipc_sockname);
    struct ipc_msg *response = reinterpret_cast<ipc_msg *>(new uint8_t[resplen]);

    response->type = msg->type;
    response->pid = pid;

    struct ipc_sockname *nameres = (struct ipc_sockname *) ((struct ipc_err *) response->data)->data;
    rc = tcp.get_name(name->socket, (struct sockaddr *) nameres->sa_data, &nameres->address_len,
                      msg->type == IPC_GETPEERNAME);

    struct ipc_err err;
    if (rc < 0) {
        err.err = -rc;
        err.rc = -1;
    } else {
        err.err = 0;
        err.rc = rc;
    }
    memcpy(response->data, &err, sizeof(struct ipc_err));
    nameres->socket = name->socket;

    if (send(fd, response, resplen, MSG_NOSIGNAL) == -1) {
        perror("Error on writing IPC getpeername response");
    }
}

void MessageListener::net_read(struct ipc_msg *msg, int fd) {
    pid_t pid = msg->pid;
    auto *payload = reinterpret_cast<struct ipc_read *>(msg->data);
    uint8_t rbuf[payload->len];
    ssize_t n = tcp.read(payload->sockfd, rbuf, payload->len);

    size_t resplen = sizeof(struct ipc_msg) + sizeof(struct ipc_err) +
                     sizeof(struct ipc_read) + payload->len;
    auto *response = reinterpret_cast<ipc_msg *>(new uint8_t[resplen]);
    auto *error = reinterpret_cast<ipc_err *>(response->data);
    auto *actual = reinterpret_cast<ipc_read *>(error->data);

    response->type = IPC_READ;
    response->pid = pid;

    error->rc = n;
    error->err = 0;

    actual->sockfd = payload->sockfd;
    actual->len = n;
    std::copy(rbuf, rbuf + payload->len, actual->buf);
    send(fd, response, resplen, MSG_NOSIGNAL);
}

void MessageListener::net_write(struct ipc_msg *msg, int fd) {
    auto *data = reinterpret_cast<ipc_write *>(msg->data);
    printf("messgae %s\n", data->buf);
    ssize_t size = tcp.write(data->sockfd, data->buf, data->len);
    reply(fd, IPC_WRITE, msg->pid, size);
}

void MessageListener::net_connect(struct ipc_msg *msg, int fd) {
    auto *data = reinterpret_cast<struct ipc_connect *>(msg->data);
    tcp.connect(data->sockfd, data->addr.sin_addr.s_addr, ntohs(data->addr.sin_port), [&](int rc) {
        reply(fd, IPC_CONNECT, msg->pid, rc);
    });
}

void MessageListener::net_poll(struct ipc_msg *msg, int fd) {
    struct ipc_poll *data = (struct ipc_poll *) msg->data;
    pid_t pid = msg->pid;
    int rc = -1;

    struct pollfd fds[data->nfds];

    for (int i = 0; i < data->nfds; i++) {
        fds[i].fd = data->fds[i].fd;
        fds[i].events = data->fds[i].events;
        fds[i].revents = data->fds[i].revents;
    }

    rc = tcp.poll(fds, data->nfds, data->timeout);

    int resplen = sizeof(struct ipc_msg) + sizeof(struct ipc_err) + sizeof(struct ipc_pollfd) * data->nfds;
    struct ipc_msg *response = reinterpret_cast<ipc_msg *>(new uint8_t[resplen]);

    response->type = IPC_POLL;
    response->pid = pid;

    struct ipc_err err;

    if (rc < 0) {
        err.err = -rc;
        err.rc = -1;
    } else {
        err.err = 0;
        err.rc = rc;
    }

    memcpy(response->data, &err, sizeof(struct ipc_err));

    struct ipc_pollfd *polled = (struct ipc_pollfd *) ((struct ipc_err *) response->data)->data;

    for (int i = 0; i < data->nfds; i++) {
        polled[i].fd = fds[i].fd;
        polled[i].events = fds[i].events;
        polled[i].revents = fds[i].revents;
    }

    send(fd, response, resplen, MSG_NOSIGNAL);
}


