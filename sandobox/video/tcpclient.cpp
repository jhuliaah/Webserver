#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <climits>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main()
{
	int		clientsocket;
	int		status;
	char	response[256];

	// create a socket
	clientsocket = socket(AF_INET, SOCK_STREAM, 0);

	/* 0 could also de IPPROTO_TCP, but it is recommended to use 0
	to select the default protocol for the given socket type */

	// specify an address for the socket
	struct sockaddr_in	server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	// connect to the server
	status = connect(	clientsocket,
						(struct sockaddr*) &server_address,
						sizeof(server_address)	);

	// check for connection error
	if (status == -1)
	{
		printf("Error connecting to the remote socket\n");
		return 1;
	}

	// receive data from the server
	recv(clientsocket, response, sizeof(response), 0);

	/* flag can be set to 0, or MSG_WAITALL, MSG_PEEK, MSG_OOB, which
	means that the function will wait for all data to be received,
	peek at the incoming data without removing it from the queue, or
	receive out-of-band data, respectively. */

	// print response	
	printf("The server sent the data: %s\n", response);

	// close the socket
	close(clientsocket);
	return 0;
}

