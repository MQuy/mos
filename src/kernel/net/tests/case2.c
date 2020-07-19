#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int server_fd, client_fd, addr_size;
	struct sockaddr_in server, client;
	char message[2000];

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd == -1)
		perror("Could not create socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(strtol(argv[1], NULL, 0));

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		perror("reuse port failed");

	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
		perror("bind failed");

	listen(server_fd, 3);

	addr_size = sizeof(struct sockaddr_in);
	client_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&addr_size);
	if (client_fd < 0)
		perror("accept failed");

	while (true)
	{
		int len = recv(client_fd, message, sizeof(message), 0);
		if (len <= 0)
			break;
		send(client_fd, message, len, 0);
	}

	return 0;
}
