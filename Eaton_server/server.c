/**
 * Server part of test application
 * It's constantly listening for incoming connections
 * Server expects messages from clients of following formst:
 * <Client name> <Measurements data>
 * Client name - null-terminated text string (char *)
 * Measurements data - numeric value (unsigned short)
 * Measurements data is out of scope of application and is ignored
 * Client name is transferred to database for storing
 * While different clients can be handled in parallel threads,
 * database update is protected from simultaneous access by mutex
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "database.h"

#define MAX_BUFFER_SIZE 256
#define MAX_NAME_LENGTH 253

static pthread_mutex_t database_mutex;

/**
 * Connection handler that serves as a thread function
*/
void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;			///< Socket handler of connected client
    int read_size;							///< Size of data chunk from the client
	int name_len;							///< Client name length (excluding null-terminator)
	int buffer_offset = 0;					///< Offset in data buffer to store new data
	unsigned short measurement_data;		///< Value of client's measurements, will be ignored
    char client_message[MAX_BUFFER_SIZE];	///< Data buffer for client message
	char client_name[MAX_NAME_LENGTH];		///< Client name text string

	/* Read data in cycle to receive whole message even if it's split into parts */
    while((read_size = recv(sock, (client_message + buffer_offset), MAX_BUFFER_SIZE, 0)) > 0)
    {
        buffer_offset += read_size;
		if (buffer_offset > MAX_BUFFER_SIZE)
		{
			break;
		}
    }

	close(sock);

	/* Check if data received correctly, otherwise discard the message */
	if((read_size < 0) || (buffer_offset > MAX_BUFFER_SIZE))
	{
		perror("Error in data exchange");
		return 0;
	}

    /* Client name must end with \0 */
	name_len = strlen(client_message);
	if(name_len > MAX_NAME_LENGTH)
	{
		puts("Corrupted data received, ignoring");
		return 0;
	}

	/* Everything before first \0 is client name */
	strcpy(client_name, client_message);
	/* And 2 bytes of client measurements */
	measurement_data = *(unsigned short *)&client_message[name_len+1];

	/* Dummy comparation to make variable "used" */
	if (measurement_data < 0)
	{
		puts("This will never happen, so we're just avoiding compiler errors");
		return 0;
	}

	/* As database is a shared resource, need to lock access to it with mutex */
	pthread_mutex_lock(&database_mutex);
	AddMeasurement(client_name);
	PrintDatabase();
	pthread_mutex_unlock(&database_mutex);

    return 0;
}

int main(int argc, char *argv[])
{
	pthread_t thread_id;					///< ID of the connection handler thread
	struct sockaddr_in server;				///< Server socket structure
	struct sockaddr_in client;				///< Client socket structure
    int socket_desc;						///< Server socket handler
	int client_sock;						///< Client socket handler
	int size = sizeof(struct sockaddr_in);	///< Socket structure size
	unsigned short port = 0;				///< Port to listen

	if (argc < 2)
	{
		puts("Usage: ./server <port>");
		puts("Please specify port number to listen for client connections");
		return -1;
	}
	else
	{
		port = atoi(argv[1]);
	}

    /* Server socket creation, setup and connection */
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
		return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Could not bind socket");
		close(socket_desc);
        return -1;
    }

    if(listen(socket_desc, 50) < 0)
	{
		perror("Could not listen to socket");
		close(socket_desc);
		return -1;
	}

	puts("Waiting for the first client to connect...");

    /* Here we have socket that expects incoming connections */
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&size)) >= 0)
    {
		/* Process each connection in separate thread */
        if(pthread_create(&thread_id, NULL, connection_handler, (void*)&client_sock) < 0)
        {
            perror("Could not create thread");
			close(socket_desc);
			close(client_sock);
            return -1;
        }
    }

	/* We get here if there's somethong wrong with connection */
    perror("Could not accept incoming connection");
	close(socket_desc);
    return -1;
}
