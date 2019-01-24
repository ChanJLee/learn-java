#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "open socket failed" << std::endl;
        return -1;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
        std::cerr << "bind failed" << std::endl;
        return -2;
    }

    if (listen(fd, 1024) < 0) {
        std::cerr << "listen failed" << std::endl;
        return -3;
    }

    std::cout << "listen, port: " << 8080 << std::endl;
    int len = 0;
    char buffer[1024] = "fuck";
    while(true) {
        int client_fd = accept(fd, nullptr, nullptr);
        write(client_fd, buffer, sizeof(buffer));
        close(client_fd);
    }

    close(fd);
    return 0;
}
