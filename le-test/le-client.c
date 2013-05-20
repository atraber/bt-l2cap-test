
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int main(int argc, char **argv)
{
    struct sockaddr_l2 addr;
    int s, status;
    char dest[18] = "D0:07:90:5B:77:06";

    if(argc < 2)
    {
        fprintf(stderr, "usage: %s <bt_addr>\n", argv[0]);
        //exit(2);
    }
    else
    {
        strncpy(dest, argv[1], 18);
    }

    // allocate a socket
    s = socket(PF_BLUETOOTH, SOCK_DGRAM, BTPROTO_L2CAP);

    if(s < 0)
    {
        perror("Cannot create socket");
        exit(2);
    }

    // set the connection parameters (who to connect to)
    memset(&addr, 0, sizeof(addr));

    addr.l2_family = AF_BLUETOOTH;

    //addr.l2_psm = htobs(0x1001);
    addr.l2_cid = htobs(0x0004);

    addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    str2ba( dest, &addr.l2_bdaddr );

    /*
    struct l2cap_options opts;
    socklen_t optlen = sizeof(int); //sizeof(struct l2cap_options);

    int opt;
    getsockopt(s, SOL_L2CAP, L2CAP_FEAT_FIXED_CHAN, &opt, &optlen);

    //opts.mode |= L2CAP_FEAT_FIXED_CHAN;
    opt = 1;

    setsockopt(s, SOL_L2CAP, L2CAP_FEAT_FIXED_CHAN, &opt, &optlen);

    getsockopt(s, SOL_L2CAP, L2CAP_FEAT_FIXED_CHAN, &opt, &optlen);*/

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if(status < 0)
    {
        perror("Could not connect");
        exit(2);
    }

    int bSend = 0;

    if(bSend)
    {

        char buffer[256];
        int i = 0;
        while(1)
        {
            sprintf(buffer, "This is %d", i++);
            printf("Sending message: %s\n", buffer);

            // send a message
            status = send(s, buffer, strlen(buffer), 0);

            printf("Status was: %d\n", status);
            if(status < 0) perror("uh oh");

            sleep(1);
        }
    }
    else
    {
        char buffer[256];

        while(1)
        {
            recv(s, buffer, 256, 0);

            printf("Status was: %d\n", status);
            if(status < 0) perror("uh oh");

            sleep(1);
        }
    }

    close(s);
}

