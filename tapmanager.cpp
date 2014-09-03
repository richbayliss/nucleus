#include "tapmanager.h"

#include <cstring>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

TapManager::TapManager()
{
    this->interfaceName = "";
}

TapManager::TapManager(std::string intf)
{
    this->interfaceName = intf;
}

std::string TapManager::getInterfaceName()
{
    return this->interfaceName;
}

int TapManager::openIntf()
{
    struct ifreq ifr;
    int err;
    const char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
     *
     * char *dev: the name of an interface (or '\0'). MUST have enough
     *   space to hold the interface name if '\0' is passed
     * int flags: interface flags (eg, IFF_TUN etc.)
     */

    /* open the clone device */
    if( (this->interface = open(clonedev, O_RDWR)) < 0)
    {
        return this->interface;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    /* We want to intialise a Layer2 tunnel so get a TAP interface */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

    /* if a device name was specified, put it in the structure; otherwise,
         * the kernel will try to allocate the "next" device of the
         * specified type */
    if (this->interfaceName != "")
    {
        strncpy(ifr.ifr_name, this->interfaceName.c_str(), IFNAMSIZ);
    }

    /* try to create the device */
    if( (err = ioctl(this->interface, TUNSETIFF, (void *) &ifr)) < 0 )
    {
        close(this->interface);
        return err;
    }

    /* if the operation was successful, write back the name of the
     * interface to the variable "dev", so the caller can know
     * it. Note that the caller MUST reserve space in *dev (see calling
     * code below) */
    this->interfaceName.assign(ifr.ifr_name);

    /* this is the special file descriptor that the caller will use to talk
     * with the virtual interface */
    return this->interface;
}

int TapManager::readInterface(unsigned char *buffer, int length)
{
    return read(this->interface, buffer, length);
}

int TapManager:: writeInterface(unsigned char *buffer, int length)
{
    return write(this->interface, buffer, length);
}
