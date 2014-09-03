#include "udpsocket.h"

#include <sys/ioctl.h>

UdpSocket::UdpSocket()
{
    this->localPort = 0;
    this->socket_fd = 0;
}

UdpSocket::UdpSocket(int port)
{
    this->localPort = port;
}

UdpSocket::~UdpSocket()
{
    //dtor
}

int UdpSocket::getSocket()
{
    /* if the socket is already available, return it */
    if (this->socket_fd != 0)
        return this->socket_fd;

    /* build the local address to use */
    char buf[5];
    sprintf(buf, "%d", this->localPort);

    const char* hostname = 0; /* wildcard */
    const char* portname = buf;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    struct addrinfo* socketConfig = 0;
    int err = getaddrinfo(hostname, portname, &hints, &socketConfig);
    if (err != 0)
    {
        std::cout << "Error. Unable to resolve local socket address (" << err << ")" << std::endl;
        return -1;
    }

    /* configure the socket type */
    this->socket_fd = socket(socketConfig->ai_family, socketConfig->ai_socktype, socketConfig->ai_protocol);
    if (this->socket_fd < 0)
    {
        std::cout << "Error. Unable to create socket." << std::endl;
        return -1;
    }

    /* make the socket non-blocking */
    //ioctl(this->socket_fd, FIONBIO, 1);

    /* bind the socket */
    int bindResult = bind(this->socket_fd, socketConfig->ai_addr, socketConfig->ai_addrlen);
    if (bindResult < 0)
    {
        std::cout << "Error. Unable to bind to port " << this->localPort << std::endl;
        return -1;
    }

    /* update our localPort */
    sockaddr_in address;
    socklen_t addressLength = sizeof(address);
    if (getsockname(this->socket_fd, (struct sockaddr *)&address, &addressLength) == -1)
    {
        std::cout << "Error. Unable to determine local port";
        return -1;
    }
    this->localPort = ntohs(address.sin_port);

    /* return it */
    return this->socket_fd;
}

int UdpSocket::sendPacket(std::string address, int port, unsigned char *data, int length)
{
    struct sockaddr_in endpoint;

    /* store this IP address in endpoint */
    inet_aton(address.c_str(), &endpoint.sin_addr);
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(port);

    /* send the packet */
    int sent_bytes =
            sendto(this->socket_fd,
                   data,
                   length,
                   0,
                   (sockaddr*)&endpoint,
                   sizeof(endpoint));

    return sent_bytes;
}

int UdpSocket::recvPacket(sockaddr_in *sender, socklen_t *senderLength, unsigned char *data, int length)
{
    return recvfrom( this->socket_fd,
                     (char*)data,
                     length,
                     0,
                     (sockaddr*)sender,
                     senderLength );
}
