#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config
{
    public:
        Config();
        virtual ~Config();

        bool isDebug() { return this->debug; }
        void setDebug(bool state) { this->debug = state; }

        int getServerPort() { return this->serverPort; }
        void setServerPort(int port) { this->serverPort = port; }

        std::string getServerAddr() { return this->serverAddr; }
        void setServerAddr(std::string address) { this->serverAddr = address; }

        int getLocalPort() { return this->localPort; }
        void setLocalPort(int port) { this->localPort = port; }

        std::string getTapInterfaceName() { return this->tapInterface; }
        void setTapInterfaceName(std::string name) { this->tapInterface = name; }

        bool loadConfigFile(std::string filename);

    protected:
    private:
        bool debug;
        int localPort, serverPort;
        std::string serverAddr, tapInterface;
};

#endif // CONFIG_H
