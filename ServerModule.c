#include "ServerModule.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>

void *server_handler(void *arg)
{
	FILE *fp = NULL;
	fp = fopen("/var/log/messages", "a+");
	fprintf(fp, "Server thread is up and running...\n");

	int newsockfd;
	int sock_fd;
	int retval;
	int yes = 1;

	struct addrinfo hints;
	struct addrinfo *p;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;				//	Use IPv4
	hints.ai_socktype = SOCK_STREAM;		//	Socket streams
	hints.ai_protocol = IPPROTO_TCP;		//	Use TCP instead of UDP
	hints.ai_flags = 0;

	retval = getaddrinfo("127.0.0.1", "1955", &hints, &servinfo);
	if(retval != 0)
		pthread_exit(NULL);

	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sock_fd < 0)
		{
			//fejl(obj->fp);
			continue;
		}

		retval = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		//if(retval < 0)
			//fejl(obj->fp);

		retval = bind(sock_fd, p->ai_addr, p->ai_addrlen);
		if(retval < 0)
		{
			close(sock_fd);
			//fejl(obj->fp);
			continue;
		}
		break;
	}

	if(p == NULL)
	{
		//fejl(obj->fp);
		pthread_exit(NULL);
		return NULL;
	}

	retval = listen(sock_fd, 5);
	if(retval < 0)
	{
		//fejl(obj->fp);
		return NULL;
	}

	while(1)
	{
		struct sockaddr client_socket;
		socklen_t addr_size;

		newsockfd = accept(sock_fd, &client_socket, &addr_size);

		pthread_t tid;
		if( pthread_create(&tid, NULL, client_handler, &newsockfd) != 0)
			return NULL;
			//fejl(obj->fp);
	}

    return NULL;
}

void *client_handler(void *arg)
{
	//serverObject *obj;
	//obj = (serverObject *)arg;
	int sockfd;
	int bytes;
	char buffer[256];
	sockfd = *(int *)arg;

	//fprintf(obj->fp, "A client has been connected.\n");
	memset(buffer, 0, 256);

	bytes = recv(sockfd, buffer, 256, 0);
	if(bytes < 0)
	{
		//fprintf(obj->fp, "Could not receive anything from client.\n");
		return NULL;
	}

	while(bytes > 0)
	{
		if( write(sockfd, buffer, strlen(buffer)) < 0)
		{
			//fprintf(obj->fp, "Could not write anything to client.\n");
			return NULL;
		}
		memset(buffer, 0, 256);
		bytes = recv(sockfd, buffer, 256, 0);
	}
		//fprintf(obj->fp, "Client exited...\n");
        return NULL;
}
