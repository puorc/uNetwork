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
                reply(fd, IPC_SOCKET, msg->pid, tcp.alloc());
                break;
            case IPC_CONNECT:
                std::cout.flush();
                auto *data = reinterpret_cast<ipc_connect *>(msg->data);
                tcp.init(data->sockfd, data->addr.sin_addr.s_addr, 80);
                reply(fd, IPC_CONNECT, msg->pid, 0);
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


