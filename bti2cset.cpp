
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>


class BTI2CPacket
{
public:
    BTI2CPacket();
    ~BTI2CPacket();
    void assemble(char* buf, unsigned int size);
    unsigned int size() const;
    bool parse(char* buf, unsigned int size);

    int slaveAddress; // I2C slave address
    bool read; // true for read, false for write
    bool request; // true if it is a request, false if it is a response
    bool error; // true if an error occurred on sending the command sequence over I2C

    unsigned char commandLength; // length of the command sequence
    char* commandBuffer; // pointer to buffer containing command sequence

    // only for i2c read response
    unsigned char readLength; // buffer length for I2C read
    char* readBuffer; // pointer to read buffer, should be != NULL for a response
};

BTI2CPacket::BTI2CPacket()
{
    this->slaveAddress = -1;
    this->read = 0;
    this->request = 1;
    this->error = 0;

    this->commandLength = 0;
    this->commandBuffer = NULL;

    this->readLength = 0;
    this->readBuffer = NULL;
}

BTI2CPacket::~BTI2CPacket()
{
    if(commandBuffer != NULL)
        free(commandBuffer);

    if(readBuffer != NULL)
        free(readBuffer);
}

unsigned int BTI2CPacket::size() const
{
    if(this->readBuffer != NULL)
        return 2 + this->readLength + this->commandLength;
    else
        return 2 + this->commandLength;
}

/**
 * @brief BTI2CPacket::assemble assumes that the param buf points to an empty buffer element where a new I2CPacket should be placed
 * @param buf
 * @param length
 */
void BTI2CPacket::assemble(char* buf, unsigned int length)
{
    buf[0] = this->read << 7 | this->slaveAddress;

    if(this->read)
    {
        // the packet is a read
        buf[1] = this->request << 7 | this->error << 6 | this->readLength;

        if(this->commandLength != 0)
            memcpy(&buf[2], this->commandBuffer, this->commandLength);

        if(!this->request)
        {
            // the packet is a response, therefore we need the read buffer
            memcpy(&buf[2 + this->commandLength], this->readBuffer, this->readLength);
        }
    }
    else
    {
        // the packet is a write
        buf[1] = this->request << 7 | this->error << 6;
        memcpy(&buf[2], this->commandBuffer, this->commandLength);
    }
}

bool BTI2CPacket::parse(char *buf, unsigned int i2cSize)
{
    if(i2cSize <= 2) // if a packet is smaller than or equal to 2 byte it cannot contain any information at all
        return false;

    this->read = (buf[0] & 0x80) != 0;
    this->slaveAddress = buf[0] & 0x7F;

    this->request = (buf[1] & 0x80) != 0;
    this->error = (buf[1] & 0x40) != 0;

    if(this->read)
    {
        // the packet is a read
        this->readLength = buf[1] & 0x1F;


        if(this->readLength + 2 > i2cSize) // the packet is too small, it cannot contain a valid response
            return false;

        this->commandLength = i2cSize - 2 - (this->request ? 0 : this->readLength);
        this->commandBuffer = (char*)malloc(this->commandLength);
        memcpy(this->commandBuffer, &buf[2], this->commandLength);

        if(!this->request)
        {
            // the packet is a response, we have to copy the read buffer
            this->readBuffer = (char*)malloc(this->readLength);
            memcpy(this->readBuffer, &buf[2 + this->commandLength], this->readLength);
        }
    }
    else
    {
        // the packet is a write
        this->commandLength = i2cSize - 2;
        this->commandBuffer = (char*)malloc(this->commandLength);
        memcpy(this->commandBuffer, &buf[2], this->commandLength);
    }

    return true;
}

void packethandler(char* buffer, unsigned int length, int socket)
{
    // parse the packet
    while(length != 0)
    {
        unsigned char type = (buffer[0] & 0xE0) >> 5;
        unsigned char packetLength = (buffer[0] & 0x1F);

        unsigned char seq = buffer[1];
        unsigned char seqAck = buffer[2];

        if(type == 0) // I2C packet
        {
            BTI2CPacket packet;
            if( !packet.parse(buffer + 3, packetLength - 3) )
            {
                printf("Parsing packet failed\n");
                return;
            }

            // do something
            packet.request = 0;
            packet.readLength = 1;
            packet.readBuffer = (char*)malloc(packet.readLength);
            packet.readBuffer[0] = seq;

            char sendBuffer[200];
            sendBuffer[0] = packet.size() + 3;
            sendBuffer[1] = 0x23;
            sendBuffer[2] = seq;
            packet.assemble(sendBuffer + 3, 200);

            printf("Sending packet\n");

            int status = write(socket, sendBuffer, sendBuffer[0]);
            if(status == -1)
                perror("Failed to send");
        }
        else if(type == 1) // gpio packet
        {
            bool req = (buffer[3] & 0x80) != 0;
            bool err = (buffer[3] & 0x40) != 0;
            unsigned char pinGroup = buffer[3] & 0x1F;

            printf("State of GPIOs is: %x\n", buffer[4]);
        }

        buffer += packetLength;
        length -= packetLength;
    }
}


int main(int argc, char **argv)
{
    struct sockaddr_l2 addr = { 0 };
    int s, status;
    char dest[18] = "E0:06:E6:BA:DA:B3";

    if(argc < 2)
    {
        fprintf(stderr, "usage: %s <bt_addr> <i2c_addr> <first_cmd> [<sec_cmd>]\n", argv[0]);
        exit(2);
    }

    strncpy(dest, argv[1], 18);

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x1001);
    str2ba( dest, &addr.l2_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if(status < 0)
    {
        perror("Could not connect");
    }

    char buffer[256];


    int i2cAddr = strtol(argv[2], NULL, 0);
    unsigned int commandLength = argc > 4 ? 2 : 1;

    char* command = (char*)malloc(commandLength);
    command[0] = strtol(argv[3], NULL, 0);

    if(argc > 4)
        command[1] = strtol(argv[4], NULL, 0);

    unsigned int readLength = 0;


    BTI2CPacket packet;
    packet.error = 0;
    packet.read = readLength == 0 ? 0 : 1;
    packet.request = 1;
    packet.slaveAddress = i2cAddr;
    packet.commandLength = commandLength;
    packet.commandBuffer = (char*)malloc(packet.commandLength);

    memcpy(packet.commandBuffer, command, packet.commandLength);

    packet.readLength = readLength;

    buffer[0] = packet.size() + 3;
    buffer[1] = 0x23;
    buffer[2] = 0xFF;
    packet.assemble(buffer + 3, 253);

    printf("Sending message\n");

    status = write(s, buffer, packet.size() + 3);

    printf("Send status was: %d\n", status);
    if(status < 0) perror("uh oh");

    status = read(s, buffer, 256);
    printf("Receive status was: %d\n", status);
    if(status < 0) perror("uh oh");

    packet.parse(buffer + 3, status - 3);

    printf("Answer to slave address %d\nError: %d\n", packet.slaveAddress, packet.error);

    printf("Command was: %d\n", packet.commandLength);
    for(unsigned int i = 0; i < packet.commandLength; i++)
        printf("0x%x ", (unsigned int)(unsigned char)packet.commandBuffer[i]);

    printf("\n");

    if(packet.read && packet.readLength != 0)
    {
        printf("Answer was: %d\n", packet.readLength);
        for(unsigned int i = 0; i < packet.readLength; i++)
            printf("0x%x ", (unsigned int)(unsigned char)packet.readBuffer[i]);

        printf("\n");
    }

    close(s);
}

