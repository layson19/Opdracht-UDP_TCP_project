#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <time.h>
	#include <stdint.h>


    #define MAX_BUF_SIZE 1024
    #define MAX_RANDOM_NUMS 42
    #define TIMEOUT 3
	#define TIMEOUT_SEC 1
    #define RETRANS_MAX 10

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
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
void cleanup( int internet_socket, struct sockaddr * internet_address );

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();

	struct sockaddr * internet_address = NULL;
	socklen_t internet_address_length = 0;
	int internet_socket = initialization( &internet_address, &internet_address_length );

	/////////////
	//Execution//
	/////////////

	execution( internet_socket, internet_address, internet_address_length );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket, internet_address );

	OSCleanup();

	return 0;
}

int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	int getaddrinfo_return = getaddrinfo( "::1", "24042", &internet_address_setup, &internet_address_result );
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
			*internet_address_length = internet_address_result_iterator->ai_addrlen;
			*internet_address = (struct sockaddr *) malloc( internet_address_result_iterator->ai_addrlen );
			memcpy( *internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			break;
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

void execution( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length )
{
	//Step 2.1
	int number_of_bytes_send = 0;

    ////////////////////////////////

    //// "GO" message to the server
    //char buffer[16] = "GO";
	// copy a message into the buffer
    number_of_bytes_send = sendto(internet_socket, "GO", 16, 0, internet_address, internet_address_length );
    if (number_of_bytes_send < 0) {
        perror( "sendto" );
	}
	else if (number_of_bytes_send < 16) {
    printf( "Sent only %d bytes of message\n", number_of_bytes_send );
}
	else {
		printf( "Sent GO message\n");
	}

    // Step 4: Receive up to MAX_NUMBERS random numbers from the server
    srand(time(NULL));
    uint32_t numbers[MAX_RANDOM_NUMS];
    int i, num_numbers = 0, retrans = 0, retrans_max;
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
	char buffer[1000];
    while (num_numbers < MAX_RANDOM_NUMS) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(internet_socket, &fds);
        int ready = select(internet_socket + 1, &fds, NULL, NULL, &timeout);
        if (ready == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (ready == 0) {
            if (++retrans > (retrans_max = rand() % RETRANS_MAX + 1)) {
                printf("Timed out waiting for response, giving up\n");
                break;
            } else {
                printf("Timed out waiting for response, retransmitting (%d/%d)\n", retrans, retrans_max);
                number_of_bytes_send = sendto(internet_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&internet_address, internet_address_length);
                if (number_of_bytes_send < 0) {
                    perror("sendto");
                    exit(EXIT_FAILURE);
                }
                timeout.tv_sec = TIMEOUT_SEC;
                timeout.tv_usec = 0;
                continue;
            }
        }
        ssize_t bytes_received = recv(internet_socket, (char*)&numbers + num_numbers, sizeof(uint32_t), 0);
        if (bytes_received < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        num_numbers++;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        retrans = 0;
    


	// Step 5: Find the largest number and send it back to the server
	int max_number = numbers[0];
	for (int i = 1; i < num_numbers; i++) {
		if (numbers[i] > max_number) {
			max_number = numbers[i];
		}
	}
    uint32_t network_byte_order_max_number = htonl(max_number);

	// Send max_number back to the server
	int num_sent = sendto(internet_socket, (char*)&network_byte_order_max_number, sizeof(network_byte_order_max_number), 0, internet_address, internet_address_length);
	if (num_sent < 0) {
		perror("sendto");
		exit(EXIT_FAILURE);
	}
	printf("Sent largest number: %d\n", max_number);
	}

    close(internet_socket);
}

void cleanup( int internet_socket, struct sockaddr * internet_address )
{
	//Step 3.2
	free( internet_address );

	//Step 3.1
	close( internet_socket );
}