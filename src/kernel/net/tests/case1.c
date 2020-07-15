#include <arpa/inet.h>	//inet_addr
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	 //strlen
#include <sys/socket.h>
#include <unistd.h>	 //write

int main(int argc, char *argv[])
{
	int server_fd, client_fd, addr_size;
	struct sockaddr_in server, client;
	char *message;

	//Create socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd == -1)
	{
		printf("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(strtol(argv[1], NULL, 0));

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		puts("reuse port failed");
		return 1;
	}

	//Bind
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("bind failed");
		return 1;
	}

	//Listen
	listen(server_fd, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	addr_size = sizeof(struct sockaddr_in);
	while (client_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&addr_size))
	{
		puts("Connection accepted");
		close(client_fd);
		puts("Connection close");
	}

	return 0;
}
