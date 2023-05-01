
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

 //Step 1.0
 int OSInit( void )
 { 
   WSADATA wsaData; 
   WSAStartup( MAKEWORD(2,0), &wsaData ); 
 }
 int OScleanup( void )
 {
    WSACleanup();
 }
 int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length);
 void execution(int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
 void cleanup(int internet_socket, struct sockaddr * internet_address );
   

int main( int argc, char * argv[] )
{
   // Initialize Winsock

  OSInit();
  
  struct sockaddr * internet_address = NULL;
  socklen_t internet_address_length = 0;
  int internet_socket = initialization( &internet_address, &internet_address_length );

  // Excution//
   execution( internet_socket, internet_address, internet_address_length );
  //Clean up //
  
  cleanup( internet_socket, internet_address );

  //Step 3.0
 
  OScleanup();




	printf(" Compiler works ! \n" );

    return 0;

}
int initialization( struct sockaddr ** internet_address, socklen_t * internet_address_length)
{
    //step 1.1
  struct addrinfo internet_address_setup;
  struct addrinfo *internet_address_result  = NULL;
  memset( &internet_address_setup, 0, sizeof internet_address_setup);
  internet_address_setup.ai_family = AF_UNSPEC;
  internet_address_setup.ai_socktype = SOCK_DGRAM;
  getaddrinfo("::1", "24042", &internet_address_setup, &internet_address_result );

  //Step 1.2
    int internet_socket;
    internet_socket = socket ( internet_address_result ->ai_family, internet_address_result ->ai_socktype,
     internet_address_result ->ai_protocol ); 

    //Step 1.3
    *internet_address_length = internet_address_result->ai_addrlen;
    *internet_address = (struct sockaddr *) malloc( internet_address_result->ai_addrlen);
    memcpy( *internet_address, internet_address_result->ai_addr, internet_address_result->ai_addrlen);

    freeaddrinfo( internet_address_result );

    return internet_socket;
 }
 void execution(int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length )
  {
     //Step 2.1
    sendto( internet_socket, "Hello UDP world!", 16, 0, internet_address, internet_address_length );

    //Step 2.2
    int number_of_bytes_received = 0;
    char buffer[1000];
    number_of_bytes_received = recvfrom( internet_socket, buffer, (sizeof buffer ) - 1, 0, internet_address, 
    &internet_address_length);
    buffer[number_of_bytes_received] = '\0';
    printf("received : %s\n", buffer);

 }
void cleanup(int internet_socket, struct sockaddr * internet_address )
{
  //Step 3.2
  free( internet_address );

  //Step 3.1
  close( internet_socket);
 }