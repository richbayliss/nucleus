#ifndef TAPMANAGER_H
#define TAPMANAGER_H

#include <string>

class TapManager
{
    public:
        TapManager();
        TapManager(std::string intf);
        int openIntf();
        std::string getInterfaceName();
        int readInterface(unsigned char *buffer, int length);
        int writeInterface(unsigned char *buffer, int length);
    protected:
    private:
        int interface;
        std::string interfaceName;
};

#endif // TAPMANAGER_H
