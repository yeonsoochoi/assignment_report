#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

void prompt() 
{
	printf("user :  ");
	fflush(stdout);
}

int main (int argc, char *argv[])
{
	int    len, rc;
	int    client_socket;
	char   send_buf[4096];
	char   recv_buf[4096];
	struct sockaddr_in   server_addr;
	struct timeval       timeout;
	fd_set origin_set, select_result;
	int max_sd;


	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <0){
		printf("socket create error\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons(4000);
	server_addr.sin_addr.s_addr= inet_addr("127.0.0.1");


	if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0){
		printf("connected error\n");
		exit(1);
	}


	FD_ZERO(&origin_set);
	FD_SET(fileno(stdin), &origin_set);
	FD_SET(client_socket, &origin_set);
	max_sd=MAX(fileno(stdin), client_socket);

	printf("connected.\n");
	printf("please put 'quit' if you want to quit\n");
	prompt();


	while(1)
	{
		memcpy(&select_result, &origin_set,sizeof(origin_set));		
		rc = select(max_sd+1 , &select_result ,NULL ,NULL , NULL);
		
		if(FD_ISSET(fileno(stdin),&select_result))
		{
			gets(send_buf);

			if(!strcmp(send_buf,"quit")){
				printf("finished\n");
				exit(1);
			}

			send(client_socket, send_buf, strlen(send_buf), 0);
			prompt();
		}
		
		else if(FD_ISSET(client_socket, &select_result))
		{
			rc = recv(client_socket, recv_buf, 4096, 0);
			if (rc < 0){ 
				printf("recv() error.\n");
				exit(1);
			}
			if (rc == 0) 
			{
				printf("disconnected\n");
				close(client_socket);
				exit(0);
			}
			
			recv_buf[rc]=0;
			printf("\r%s\n", recv_buf);
			prompt();
		}

	}
	close(client_socket);
	return 0;
}
