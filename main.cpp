#include <iostream>
#include <cstdio>
#include <stdint-gcc.h>
#include <map>

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "config.h"
#include "tapmanager.h"
#include "udpsocket.h"

#define BUFSIZE 2000

using namespace std;

struct endpoint
{
    string address;
    int port;
};
typedef std::map<uint64_t, endpoint> MACTable;

uint64_t mtoui64(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
    return
            uint64_t(a) << 40 |
                           uint64_t(b) << 32 |
                           uint64_t(c) << 24 |
                           uint64_t(d) << 16 |
                           uint64_t(e) << 8 |
                           uint64_t(f);
}

int main(int argc, char* argv[])
{
    /* create a map of MAC addresses to endpoints */
    MACTable endpoints;

    /* load the config options file */
    Config config;
    if (argc == 2)
        if (!config.loadConfigFile(argv[1]))
        {
            cout << "Error. Unable to parse the config file." << endl;
            return -1;
        }

    /* print a welcome message */
    if (config.isDebug())
        cout << "== nucleus v0.0.1 ==" << endl;

    /* bind the TAP interface */
    TapManager tapManager(config.getTapInterfaceName());
    int tap_fd = tapManager.openIntf();

    /* if we have an issue binding to the TAP interface then inform the user and exit */
    if (tap_fd < 0)
    {
        cout << "Error: Unable to create or bind to TAP interface." << endl;
        return -1;
    }

    /* bind to the UDP socket */    
    UdpSocket udpSocket(config.getLocalPort());
    int socket_fd = udpSocket.getSocket();

    /* if we have an issue binding the socket then inform the user and exit */
    if (socket_fd < 0)
    {
        cout << "Error: Unable to create or bind to UDP socket." << endl;
        return -1;
    }

    /* inform the user we are in client/server mode */
    if (config.isDebug())
        cout << (config.getServerAddr() != "" ? "Client Mode" : "Server Mode") << endl;

    /* if we are in client mode then dump the IP:Port */
    if (config.isDebug() && config.getServerAddr()!="")
        cout << "[D] Server @ " << config.getServerAddr() << ":" << config.getServerPort() << endl;

    /* show our local UDP port */
    if (config.isDebug())
        cout << "[D] Listen: " << udpSocket.getLocalPort() << endl;

    /* inform the user of our TAP interface name */
    if (config.isDebug())
        cout << "[D] Intf: \"" << tapManager.getInterfaceName() << "\"" << endl;

    /* setup the select() loop */
    int maximumFd = (tap_fd > socket_fd) ? tap_fd : socket_fd;
    fd_set masterFds, tempFds;

    FD_ZERO(&masterFds);
    FD_SET(tap_fd, &masterFds);
    FD_SET(socket_fd, &masterFds);

    unsigned char buffer[BUFSIZE];
    int ret, bytesIn, bytesOut;
    char addr[18];
    uint64_t destMAC;

    while(true)
    {
        /* reinstate the FDs for this loop */
        tempFds = masterFds;

        /* check for any waiting data */
        ret = select(maximumFd + 1, &tempFds, NULL, NULL, NULL);

        /* if we have nothing to read and no error - loop around */
        if (ret < 0 && errno == EINTR)
        {
            continue;
        }

        /* oops! */
        if (ret < 0)
        {
            cout << "Error. Issue with select() method." << endl;
            return -1;
        }

        /* check and read the TAP interface */
        if(FD_ISSET(tap_fd, &tempFds))
        {
            /* read the ethernet frame */
            bytesIn = tapManager.readInterface(buffer, BUFSIZE);

            /* determine the destination */
            destMAC = mtoui64(buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

            /* debug output of the packet */
            if (config.isDebug())
            {
                snprintf(addr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                cout << "[D] " << bytesIn << " bytes for \"" << (destMAC == 0xffffffffffff ? "BROADCAST" : addr) << "\" on " << tapManager.getInterfaceName() << endl;
            }

            /* send the UDP packet to the endpoint */
            if (config.getServerAddr()=="")
            {
                if (destMAC == 0xffffffffffff)
                {
                    /* send to all known endpoints */
                    for(MACTable::iterator i = endpoints.begin(); i != endpoints.end(); ++i)
                    {
                        bytesOut = udpSocket.sendPacket(i->second.address, i->second.port, buffer, bytesIn);
                        if (bytesOut < bytesIn)
                            cout << "Error: Didn't send all bytes to " << i->second.address << ":" << i->second.port << endl;
                    }
                }
                else
                {
                    /* send to known endpoint only */
                    if (endpoints.count(destMAC))
                    {
                        bytesOut = udpSocket.sendPacket(endpoints[destMAC].address, endpoints[destMAC].port, buffer, bytesIn);
                        if (bytesOut < bytesIn)
                            cout << "Error: Didn't send all bytes to " << endpoints[destMAC].address << ":" << endpoints[destMAC].port << endl;
                    }
                }
            }
            else
            {
                /* send to the server endpoint */
                bytesOut = udpSocket.sendPacket(config.getServerAddr(), config.getServerPort(), buffer, bytesIn);
                if (bytesOut < bytesIn)
                    cout << "Error: Didn't send all bytes to " << config.getServerAddr() << ":" << config.getServerPort() << endl;
            }
        }

        /* check and read the UDP interface */
        if (FD_ISSET(socket_fd, &tempFds))
        {
            sockaddr_in sender;
            socklen_t senderLength;

            bytesIn = udpSocket.recvPacket(&sender, &senderLength, buffer, BUFSIZE);

            /* grab the MAC */
            destMAC = mtoui64(buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);

            /* update the MACTable if required */
            if (config.getServerAddr() == "")
            {
                char source[16] = {0};
                inet_ntop(AF_INET, &sender.sin_addr.s_addr, source, sizeof source);

                endpoint ep;
                ep.address = source;
                ep.port = ntohs(sender.sin_port);

                if (endpoints.count(destMAC) == 0 && config.isDebug()) {
                    snprintf(addr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                    cout << "[D] Updating MAC table for \"" << addr << "\" to " << ep.address << ":" << ep.port << endl;
                }

                endpoints[destMAC] = ep;
            }

            if (config.isDebug())
            {                
                snprintf(addr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

                cout << "[D] " << bytesIn << " bytes for \"" << (destMAC == 0xffffffffffff ? "BROADCAST" : addr) << "\" on UDP" << endl;
            }

            /* write the bytes out the TAP interface */
            bytesOut = tapManager.writeInterface(buffer, bytesIn);

        }
    }

    /* exit cleanly */
    return 0;
}
