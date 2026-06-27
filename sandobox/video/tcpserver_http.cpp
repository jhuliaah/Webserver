#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <climits>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main()
{
	//open a file to serve
	FILE *html_data;
	html_data = fopen("index.html", "r");

	char response_data[5000000];
	fgets(response_data, sizeof(response_data), html_data);

	char http_header[5000000] = "HTTP/1.1 200 OK\r\n\n";
	strcat(http_header, response_data);

	// create a socket
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// define the server address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	// bind the socket to our specified IP and port
	bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

	// listen for connections
	listen(server_socket, SOMAXCONN);

	int client_socket;
	while (1)
	{
		client_socket = accept(server_socket, NULL, NULL);

		// send the message to the client
		send(client_socket, http_header, sizeof(http_header), 0);

		// close the client socket
		close(client_socket);
	}
	
	return 0;
}
