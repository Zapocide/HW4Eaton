/**
 * Client part of test application
 * Infinite loop that creates client messages of following format:
 * <Client name> <Measurements data>
 * Client name - null-terminated text string (char *)
 * Measurements data - numeric value (unsigned short)
 * Real-time clock is used to generate client names and measurements
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    int sock = 0;					///< Socket for server connection
	unsigned short port = 0;		///< Port to connect
    struct sockaddr_in serv_addr;	///< Structure for socket API
    char client_name[10];			///< Holds generated client name
	unsigned short meas;			///< Holds generated measurement data
	struct timespec timer;			///< Timer structure to generate client names and measurements

	if (argc < 2)
	{
		puts("Usage: ./client <port>");
		puts("Please specify port number to connect to server");
		return -1;
	}
	else
	{
		port = atoi(argv[1]);
	}

	/* Main loop to generate client messages */
	while(1)
	{
		/* Sockets creation, setup and connection */
	    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        perror("Could not create socket");
	        break;
	    }

	    memset(&serv_addr, 0, sizeof(serv_addr));

	    serv_addr.sin_family = AF_INET;
	    serv_addr.sin_port = htons(port);

	    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	    {
	        perror("Invalid address");
			close(sock);
			break;
	    }

	    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	    {
	        perror("Connection failed");
			close(sock);
			break;
	    }

		if (clock_gettime(CLOCK_REALTIME, &timer) < 0)
		{
			perror("Could not get current time");
			close(sock);
			break;
		}

		/* Take 1 digit from nanoseconds measured by timer to generate client name */
		sprintf(client_name, "Client%ld", ((timer.tv_nsec / 100) % 10));
		/* Seconds elapsed from startup will serve as a measurement data */
		meas = timer.tv_sec;

		/* Sending data to the server */
	    if (send(sock, client_name, strlen(client_name)+1, 0) < 0)
		{
			perror("Failed to send client name");
			close(sock);
			break;
		}
		if (send(sock, (void*)&meas, sizeof(meas), 0) < 0)
		{
			perror("Failed to send measurements");
			close(sock);
			break;
		}

		printf("%s sent data %d\n", client_name, meas);
		memset(client_name, 0, sizeof(client_name));

		close(sock);
		sleep(1);
	}

    return 0;
} 
