// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"
#include "macros.h"

int fd;

LinkLayer layer;

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    strcpy(layer.serialPort, serialPort);
    if (role == "tx"){
        layer.role = LlTx;
    }
    else if(role == "rx"){
        layer.role = LlRx;
    }

    layer.baudRate = baudRate;
    layer.nRetransmissions = nTries;
    layer.timeout = timeout;

    fd = llopen(layer);

    unsigned char buffer[3];

    buffer[0] = FLAG;
    buffer[1] = ESC;
    buffer[2] = ESC;

}
