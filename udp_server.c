#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <stdint.h>
	#include <stdbool.h>

    #define MAX_BUF_SIZE 1024
    #define MAX_RANDOM_NUMS 42
    #define TIMEOUT 3
	#define TIMEOUT_SEC 3
	#define RETRANSMISSION_MAX 10
	#define RETRANSMISSION_INTERVAL_SEC 1

	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	int OSInit( void ) {}
	int OSCleanup( void ) {}
#endif

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket);

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	int internet_socket = initialization();

	/////////////
	//Execution//
	/////////////

	execution( internet_socket );
	
	


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket);

	OSCleanup();

	return 0;
}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				close( internet_socket );
				perror( "bind" );
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

void execution( int internet_socket )
{
	//Step 2.1
	int number_of_bytes_received = 0;
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;

	// Wait for a "GO" message from the client
	number_of_bytes_received = recvfrom(internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
	if (number_of_bytes_received == -1) {
		perror("Error receiving message");
	}

	buffer[number_of_bytes_received] = '\0';
	if (strcmp(buffer, "GO") != 0) {
        //printf("errno = %d\n", errno);
		printf("Invalid message received from client: %s\n", buffer);

	}
	else {
		printf("Received \"GO\" message\n");
	}

	// Generate and send random numbers to the client
	// Step 4: Send max 42 random numbers in network byte order
    printf("Sending random numbers to client...\n");
	// Generate 42 random numbers and send them to the client
int i;
for (i = 0; i < 42; i++) {
    uint32_t random_number = htonl(rand());
    char buffer[sizeof(random_number)];
    memcpy(buffer, &random_number, sizeof(random_number));
    int number_of_bytes_sent = sendto(internet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_internet_address, client_internet_address_length);
    if (number_of_bytes_sent == -1) {
        perror("Error sending message");
    }
}

int retransmission_count = 0;
int largest_number = 0;

while (retransmission_count <= RETRANSMISSION_MAX) {

	// Step 5: Receive largest number from client
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(internet_socket, &read_fds);

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	int select_status = select(internet_socket + 1, &read_fds, NULL, NULL, &timeout);

	if (select_status < 0) {
		perror("Error in select");
		continue;
	}
	else if (select_status == 0) {
		printf("Timeout waiting for response from client\n");
		retransmission_count++;
		continue;
	}
	else {
		uint32_t network_byte_order_largest_number; 
		int number_of_bytes_received = recvfrom(internet_socket, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_internet_address, &client_internet_address_length);
}
    close( internet_socket );   
}
}

void cleanup( int internet_socket )
{
	//Step 3.1
	close( internet_socket ); 
}
