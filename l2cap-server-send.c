
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int main(int argc, char **argv)
{
    struct sockaddr_l2 loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // bind socket to port 0x1001 of the first available 
    // bluetooth adapter
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_psm = htobs(0x1001);

    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str( &rem_addr.l2_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client, &readfds);

    char buffer[200];
    int status;
    int i = 0;
    do
    {
	sprintf(buffer, "This is %d", i++);
	printf("Sending message: %s\n", buffer);
        // send a message

        status = send(client, buffer, strlen(buffer), 0);

	printf("Status was: %d\n", status);
        if(status < 0) perror("uh oh");

	sleep(1);
    }
    while(1);

    // close connection
    close(client);
    close(s);
}
