
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define THREADS 2
#define PORT 1955

typedef struct Thr_Arg
{
	pthread_t tid[THREADS];
	int socket_file_desciptor;
	int new_socket_file_descriptor;
	int bytes;
	socklen_t size;
	struct sockaddr_in server_adr;
	struct sockaddr_in client_adr;
	char buffer[256];
	char msg[50];
	FILE *file_p;
} thr_arg;

void error(FILE *file_p)	{
	fprintf(file_p, "Error occured.\n");
	exit(1);	}

void signal_handler(int signo)
{ exit(0); }

void *server_handler(void *arg)
{
	thr_arg *myData;
	myData = (thr_arg *) arg;
	memset(myData->msg, '\0', sizeof(myData->msg));
	strcpy(myData->msg, "Message received\n");

	fprintf(myData->file_p, "Server pending...\n");
	listen(myData->socket_file_desciptor, THREADS);

	for(;;)
	{
		sleep(1);

		myData->size = sizeof(myData->client_adr);
		bzero(myData->buffer, 256);

		myData->new_socket_file_descriptor = accept(myData->socket_file_desciptor, (struct sockaddr *) &myData->client_adr, &myData->size);
		if(myData->new_socket_file_descriptor < 0)
			error(myData->file_p);

		else
		{
			myData->bytes = read(myData->new_socket_file_descriptor, myData->buffer, 255);
			if(myData->bytes < 0)
				error(myData->file_p);

			while(myData->bytes > 0)
			{
				fprintf(myData->file_p, "Message received from client: %s", myData->buffer);
				bzero(myData->buffer, 256);

				fflush(myData->file_p);
				myData->bytes = write(myData->new_socket_file_descriptor, myData->msg, sizeof(myData->msg));
				if(myData->bytes < 0)
					error(myData->file_p);

				myData->bytes = read(myData->new_socket_file_descriptor, myData->buffer, 255);
				if(myData->bytes < 0)
					error(myData->file_p);
			}
			fprintf(myData->file_p, "Client left session.\n");
			close(myData->new_socket_file_descriptor);
		}
	}

	pthread_exit(NULL);
}

int main(void)
{
	thr_arg mInput;
	int rc;

	int sock_fd, port = 0;
	struct sockaddr_in server_addr;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 2)
		error(stderr);

	bzero((char *)&server_addr, sizeof(server_addr));
	port = PORT;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		error(stderr);

	printf("Daemon is running...\n");
	if(signal(SIGTERM, signal_handler) == SIG_ERR)
		error(stderr);

	FILE *fp = NULL;
	pid_t process_id = 0;
	pid_t session_id = 0;
	process_id = fork();	//	Let the parents are have some fun...

	if (process_id < 0) {
		printf("fork failed!\n");
		exit(1);
	}
	if (process_id > 0) {
		printf("[PID] of the child process: [%d] \n", process_id);
		exit(0);
	}

	umask(0);	//	Sat til file mode - minder meget om chmod, men arbejder med nye skabte filer i stedet, herefter gælder umask ikke længere...
	session_id = setsid();
	if (session_id < 0)
		exit(1);

	chdir("/");
	close(0);
	close(1);
	close(2);

	fp = fopen("/var/log/messages", "a+");	//	Syslog

	//	Udfyld struct:
	mInput.socket_file_desciptor = sock_fd;
	mInput.file_p = fp;

	rc = pthread_create(&mInput.tid[0], NULL, server_handler, (void *)&mInput);	//	Opretter tråd til at lytte på port 1955
	if(rc != 0)
		error(fp);

	pthread_join(mInput.tid[0], NULL);
	fclose(fp);
	close(sock_fd);
	pthread_exit(NULL);
}
