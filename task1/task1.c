
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

#include "ServerModule.h"

void signal_handler(int signo)
{exit(0);}

int main(void)
{
		pthread_t tid;
		FILE *fp = NULL;
        printf("Daemon is running...\n");
        if(signal(SIGTERM, signal_handler) == SIG_ERR)
        	fprintf(fp, "Error from signal().\n");

        pid_t process_id, session_id;
        process_id = fork();

        if (process_id < 0)
        {
                printf("fork failed!\n");
                exit(1);
        }

        if (process_id > 0)
        {
                printf("[PID] of the child process: [%d] \n", process_id);
                exit(0);
        }

        umask(0);
        session_id = setsid();
        if (session_id < 0)
                exit(1);

        chdir("/");
        close(0);
        close(1);
        close(2);

        fp = fopen("/var/log/messages", "a+");

        if( pthread_create(&tid, NULL, server_handler, NULL) != 0)
        	fprintf(fp, "Couldn't create thread\n");

        pthread_join(tid, NULL);
        fclose(fp);	//	close the logfile
        return 0;
}
