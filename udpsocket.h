#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <string>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

class UdpSocket
{
    public:
        UdpSocket();
        UdpSocket(int port);
        virtual ~UdpSocket();
        int getLocalPort() { return localPort; }
        int getSocket();
        int sendPacket(std::string address, int port, unsigned char *data, int length);
        int recvPacket(sockaddr_in *sender, socklen_t * senderLength, unsigned char *data, int length);
    protected:
    private:
        int localPort;
        int socket_fd;
};

#endif // UDPSOCKET_H
