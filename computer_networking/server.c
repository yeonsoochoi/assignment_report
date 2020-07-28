#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

fd_set origin_set;
int server_socket;
int max_sd;

void init_set()
{
	FD_ZERO(&origin_set);
	max_sd=0;
}

void add_to_set(int new_sd)
{
	FD_SET(new_sd, &origin_set);
	max_sd=MAX(new_sd, max_sd);
}

void remove_from_set(int sd)
{
	FD_CLR(sd, &origin_set);
	if (sd == max_sd)
	{
		while (FD_ISSET(max_sd, &origin_set) == 0)
			max_sd -= 1;
	}
}

void broadcast(int sender, char* msg)
{
	int j;
	int len=strlen(msg);
	for (j=0; j <= max_sd  ; ++j)
	{
		if (FD_ISSET(j, &origin_set) && j!=server_socket && j!=sender)
		{
			send(j, msg, len, 0);
		}
	}
	printf("Broadcasting '%s' finished\n" , msg);
}

int main (int argc, char *argv[])
{
	int    i, j, len, rc, on = 1;
	int    new_sd;
	int    close_conn;
	char   buffer[4096];
	char   buffer2[4096];
	struct sockaddr_in   server_addr;
	fd_set select_result;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket < 0) {
		printf("create socket error\n");
		exit(1);
	}
	rc = setsockopt(server_socket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        printf("setsockopt error\n");
        exit(1);
    }

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port        = htons(4000);
	
	printf("server start\n");


	rc = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

	if (rc < 0){
		printf("bind() error\n");
		exit(1);
	}

	rc = listen(server_socket, 5);
	if (rc < 0) {
		printf("listen() error\n");
		exit(1);
	}


	init_set();
	add_to_set(server_socket);

	while(1)
	{
		memcpy(&select_result, &origin_set, sizeof(origin_set));

		rc = select(max_sd + 1, &select_result, NULL, NULL, NULL);

		if (rc < 0){ 
			printf("select() error\n");
			exit(1);
		}
	
		for (i=0; i <= max_sd  ; ++i)
		{
			if (FD_ISSET(i, &select_result))
			{
				if (i == server_socket)
				{
					new_sd = accept(server_socket, NULL, NULL);
					if (new_sd < 0){ 
						printf("accept() error\n");
						exit(1);
					}
					printf("client %d connected\n", new_sd);
					add_to_set(new_sd);
					snprintf(buffer, 4096, "[%d] entered room", new_sd);
					broadcast(new_sd, buffer);
				}
				else
				{
					rc = recv(i, buffer, 4096, 0);

					if (rc < 0){ 
						printf("recv() error\n");
						exit(-9);
					}

					if (rc == 0)
					{
						close(i);
						remove_from_set(i);
						snprintf(buffer2, 4096, "client %d disconnected\n", i);
					}
					else
					{

						buffer[rc]=0; 
						snprintf(buffer2, 4096, "client %d :  %s", i, buffer);
					}

					broadcast(i, buffer2);
				
					
				} 
			} 
		} 
	} 

	for (i=0; i <= max_sd; ++i)
	{
		if (FD_ISSET(i, &origin_set))
			close(i);
	}
}

